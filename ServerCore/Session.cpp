#include "pch.h"
#include "Session.h"

Session::Session(ServiceRef service) :
	IocpObject(service), session_id_(-1), is_connected_(false), is_register_send_(false), recv_buffer_(2048)
{
	CreateSocket();
	InitializeCriticalSection(&lock_send_queue_);
}

Session::~Session()
{
	if (is_connected_ || is_register_send_)
	{
		CRASH("WRONG SESSION DESTROY");
	}

	SocketUtils::Close(socket_);
}

void Session::Start()
{
	if (socket_ != INVALID_SOCKET)
	{
		ProcessConnect();
	}
}

void Session::Close()
{
	if (socket_ != INVALID_SOCKET)
	{
		Disconnect();
	}
}

void Session::CreateSocket()
{
	socket_ = SocketUtils::CreateSocket();
}

void Session::Dispatch(IocpEvent* iocp_event, int num_of_bytes)
{
	switch (iocp_event->GetType())
	{
	case IocpEventType::RECV:
		ProcessRecv(num_of_bytes);
		break;
	case IocpEventType::SEND:
		ProcessSend(num_of_bytes);
		break;
	case IocpEventType::DISCONNECT:
		ProcessDisconnect();
		break;
	default:
		CRASH("INVALID IOCP EVENT");
	}
}

/******************************
*     Connect & Disconnect
*******************************/

void Session::ProcessConnect()
{
	IocpObject::Initialize();

	is_connected_.store(true);
	OnConnect();
	RegisterRecv();
}

void Session::Disconnect()
{
	if (is_connected_.exchange(false) == false)
	{
		return;
	}

	RegisterDisconnect();
}

void Session::RegisterDisconnect()
{
	iocp_event_disconnect_.SetOwner(shared_from_this());
	iocp_event_disconnect_.ZeroMemoryOverlapped();

	if (SocketUtils::DisconnectEx(socket_, (LPWSAOVERLAPPED)&iocp_event_disconnect_, TF_REUSE_SOCKET, 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "DisconnectEx Error (" << WSAGetLastError() << ")" << std::endl;
		}
	}
}

void Session::ProcessDisconnect()
{
	iocp_event_disconnect_.ResetOwner();

	OnDisconnect();
	service_->DestroySession(GetSessionRef());
}

/*********************
*	     Recv
**********************/

void Session::RegisterRecv()
{
	if (is_connected_ == false)
	{
		return;
	}

	iocp_event_recv_.SetOwner(shared_from_this());
	iocp_event_recv_.ZeroMemoryOverlapped();

	recv_buffer_.CleanUp();

	iocp_event_recv_.wsabuf_.buf = recv_buffer_.GetBufferRear();
	iocp_event_recv_.wsabuf_.len = recv_buffer_.GetSpace();

	DWORD num_of_bytes = 0, flags = 0;
	if (WSARecv(socket_, &iocp_event_recv_.wsabuf_, 1, &num_of_bytes, &flags, (LPWSAOVERLAPPED)&iocp_event_recv_, nullptr) == SOCKET_ERROR)
	{
		int error_code = WSAGetLastError();
		if (error_code != WSA_IO_PENDING)
		{
			iocp_event_recv_.ResetOwner();
			SocketIOErrorHandler(error_code);
		}
	}
}

void Session::ProcessRecv(int num_of_bytes)
{
	// std::cout << num_of_bytes << "recvd" << std::endl;
	iocp_event_recv_.ResetOwner();

	if (num_of_bytes == 0)
	{
		Close();
		return;
	}

	if (recv_buffer_.OnWrite(num_of_bytes) == false)
	{
		std::cout << "RecvBuffer OnWrite Error" << std::endl;
		Close();
		return;
	}

	int process_length = OnRecv(recv_buffer_.GetDataSegment(), recv_buffer_.GetLength());
	if (process_length < 0 || process_length > recv_buffer_.GetLength())
	{
		std::cout << "Wrong Process Recv Data" << std::endl;
		Close();
		return;
	}

	if (recv_buffer_.OnRead(process_length) == false)
	{
		std::cout << "RecvBuffer OnWrite Error" << std::endl;
		Close();
		return;
	}

	RegisterRecv();
}

// Echo Test
int Session::OnRecv(char* buffer, int length)
{
	/*
	SendBufferRef send_buffer = ObjectPool<SendBuffer>::MakeShared(buffer, length, length);
	Send(send_buffer);	// Echo
	*/

	SendBufferRef send_buffer = ObjectPool<SendBuffer>::MakeShared();
	send_buffer->Open(length);
	memcpy_s(send_buffer->GetBufferRear(), length, buffer, length);
	send_buffer->OnWrite(length);
	send_buffer->Close();
	Send(send_buffer);	// Echo

	delete buffer;
	return length;
}

/*********************
*	     Send
**********************/

bool Session::Send(SendBufferRef send_buffer)
{
	if (is_connected_ == false)
	{
		return false;
	}

	bool register_send = false;

	{
		EnterCriticalSection(&lock_send_queue_);
		send_queue_.push(send_buffer);
		if (is_register_send_.exchange(true) == false)
		{
			register_send = true;
		}
		LeaveCriticalSection(&lock_send_queue_);
	}

	if (register_send)
	{
		RegisterSend();
	}

	return true;
}

void Session::RegisterSend()
{
	iocp_event_send_.SetOwner(shared_from_this());
	iocp_event_send_.ZeroMemoryOverlapped();

	{
		EnterCriticalSection(&lock_send_queue_);

		while (!send_queue_.empty())
		{
			SendBufferRef buffer = send_queue_.front();
			send_queue_.pop();

			// for reference count
			iocp_event_send_.send_buffer_list_.push_back(buffer);
		}

		LeaveCriticalSection(&lock_send_queue_);
	}

	for (SendBufferRef buffer : iocp_event_send_.send_buffer_list_)
	{
		WSABUF wsabuf;
		wsabuf.buf = buffer->GetBufferFront();
		wsabuf.len = buffer->GetLength();
		// std::cout << buffer->GetLength() << "send" << std::endl;
		iocp_event_send_.wsabufs_.push_back(wsabuf);
	}

	DWORD num_of_bytes = 0, flags = 0;
	if (WSASend(socket_, iocp_event_send_.wsabufs_.data(), iocp_event_send_.wsabufs_.size(), &num_of_bytes, flags, (LPWSAOVERLAPPED)&iocp_event_send_, nullptr) == SOCKET_ERROR)
	{
		int error_code = WSAGetLastError();
		if (error_code != WSA_IO_PENDING)
		{
			iocp_event_send_.ResetOwner();
			SocketIOErrorHandler(error_code);
		}
	}
}

void Session::ProcessSend(int num_of_bytes)
{
	iocp_event_send_.ResetOwner();
	iocp_event_send_.send_buffer_list_.clear();
	iocp_event_send_.wsabufs_.clear();

	if (num_of_bytes == 0)
	{
		Close();
		return;
	}

	OnSend(num_of_bytes);

	{
		EnterCriticalSection(&lock_send_queue_);
		if (send_queue_.empty())
		{
			is_register_send_.store(false);
			LeaveCriticalSection(&lock_send_queue_);
		}
		else
		{
			LeaveCriticalSection(&lock_send_queue_);
			RegisterSend();
		}
	}
}