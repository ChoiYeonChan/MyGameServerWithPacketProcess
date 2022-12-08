#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

Session::Session(ServiceRef service) : IocpObject(service), recv_buffer_(1000),
is_connected_(true), is_register_send_(false)
{
	socket_ = SocketUtils::CreateSocket();
	InitializeCriticalSection(&lock_send_queue_);
}

Session::~Session()
{
	SocketUtils::Close(socket_);
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
		CRASH("invalid event type");
	}
}

/************************************
*	     Connect & Disconnect
*************************************/

void Session::ProcessConnect()
{
	is_connected_.store(true);
	service_->RegisterForIocp(this);	// Register Session For Iocp

	RegisterRecv();
}

void Session::Disconnect()
{
	if (is_connected_.exchange(false) == false)
		return;

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
	iocp_event_recv_.SetOwner(shared_from_this());
	iocp_event_recv_.ZeroMemoryOverlapped();
		
	iocp_event_recv_.wsabuf_.buf = recv_buffer_.GetBufferRear();
	iocp_event_recv_.wsabuf_.len = recv_buffer_.GetSpace();
	DWORD num_of_bytes = 0, flags = 0;
	if (WSARecv(socket_, &iocp_event_recv_.wsabuf_, 1, &num_of_bytes, &flags, (LPWSAOVERLAPPED)&iocp_event_recv_, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != 997)
		{
			std::cout << "WSARecv Error (" << WSAGetLastError() << ")" << std::endl;
		}
	}
}

void Session::ProcessRecv(int num_of_bytes)
{
	iocp_event_recv_.ResetOwner();

	if (num_of_bytes == 0)
	{
		Disconnect();
		return;
	}

	if (recv_buffer_.OnWrite(num_of_bytes) == false)
	{
		CRASH("SESSION RECV BUFFER ERROR");
	}

	OnRecv(recv_buffer_.GetBufferFront(), recv_buffer_.GetLength());
	recv_buffer_.CleanUp();
	RegisterRecv();
}

// Echo Test
int Session::OnRecv(char* buffer, int length)
{
	SendBufferRef send_buffer = ObjectPool<SendBuffer>::MakeShared(2048);

	if (!recv_buffer_.Read(send_buffer->GetBufferFront(), length))
	{
		recv_buffer_.Display();
		CRASH("SESSION RECV BUFFER ERROR");
	}

	send_buffer->OnWrite(length);
	Send(send_buffer);	// Echo			

	return length;
}

/*********************
*	     Send
**********************/

bool Session::Send(SendBufferRef send_buffer)
{
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
			SendBufferRef buffer;
			buffer = send_queue_.front();
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
		iocp_event_send_.wsabufs_.push_back(wsabuf);
	}

	DWORD num_of_bytes = 0, flags = 0;
	if (WSASend(socket_, iocp_event_send_.wsabufs_.data(), iocp_event_send_.wsabufs_.size(), &num_of_bytes, flags, (LPWSAOVERLAPPED)&iocp_event_send_, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "WSASend Error (" << WSAGetLastError() << ")" << std::endl;
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
		Disconnect();
		return;
	}

	OnSend();

	EnterCriticalSection(&lock_send_queue_);
	{
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

/************************
*	  PacketSession
*************************/

PacketSession::PacketSession(ServiceRef service) : Session(service)
{

}

PacketSession::~PacketSession()
{

}

int PacketSession::OnRecv(char* buffer, int length)
{
	int process_length = 0;

	while (true)
	{
		int data_size = length - process_length;
		if (data_size < sizeof(PacketHeader))
		{
			break;
		}

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[process_length]));
		// 아직 데이터가 헤더에 기록된 사이즈만큼 도착하지 않음
		if (data_size < header.size)
		{
			break;
		}

		OnRecvPacket(&buffer[process_length], header.size);

		process_length += header.size;
	}

	return process_length;
}