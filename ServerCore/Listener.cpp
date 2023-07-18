#include "pch.h"
#include "Listener.h"

void Listener::Start()
{
	listen_socket_ = SocketUtils::CreateSocket();
	if (listen_socket_ == INVALID_SOCKET)
	{
		std::cout << "[Listener] 리슨 소켓 생성에 실패했습니다." << std::endl;
		return;
	}

	IocpObject::Initialize();

	int accept_count = 8;
	for (int i = 0; i < accept_count; i++)
	{
		IocpEventAccept* accept_event = new IocpEventAccept(shared_from_this());
		accept_event_list_.push_back(accept_event);
	}

	StartAccept();
}

void Listener::Close()
{
	for (IocpEventAccept* accept_event : accept_event_list_)
	{
		delete accept_event;
	}

	accept_event_list_.clear();
	CloseSocket();
}

bool Listener::StartAccept()
{
	if (SocketUtils::SetLinger(listen_socket_, 0, 0) == false)
		return false;

	if (SocketUtils::SetReuseAddress(listen_socket_, true) == false)
		return false;

	SocketUtils::Bind(listen_socket_, service_->GetNetAddress());
	SocketUtils::Listen(listen_socket_);

	for (IocpEventAccept* accept_event : accept_event_list_)
	{
		RegisterAccept(accept_event);
	}

	return true;
}

void Listener::RegisterAccept(IocpEventAccept* accept_event)
{
	accept_event->ZeroMemoryOverlapped();

	// 일반적으로 Accept가 반환하는 소켓으로 세션을 생성하지만
	// AcceptEx는 클라이언트 소켓과 최초 데이터 수신 버퍼를 인자로 받으므로 세션을 미리 생성한다.
	SessionRef session = service_->CreateSession();
	if (session == nullptr)
	{
		std::cout << "[Listener] 세션 생성에 실패했습니다." << std::endl;
		return;
	}

	accept_event->accept_session_ = session;
	DWORD bytes_received;

	if (!SocketUtils::AcceptEx(listen_socket_, session->GetSessionSocket(), session->recv_buffer_.GetBufferFront(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes_received, (LPOVERLAPPED)(accept_event)))
	{
		int error_code = WSAGetLastError();
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			SocketIOErrorHandler(error_code);
			RegisterAccept(accept_event);
		}
	}
}

void Listener::ProcessAccept(IocpEventAccept* accept_event)
{
	SessionRef session = accept_event->accept_session_;

	// Accept와 달리 AcceptEx는 클라이언트 소켓을 미리 생성해둔 상태이므로 ListenSocket의
	// 속성을 상속하지 않는다. 따라서 별도로 속성을 상속하도록 지정해주어야 한다.
	if (SocketUtils::SetUpdateAcceptContext(session->GetSessionSocket(), listen_socket_) == false)
	{
		RegisterAccept(accept_event);
		return;
	}

	SOCKADDR_IN client_address;
	int size_of_address = sizeof(client_address);
	if (getpeername(session->GetSessionSocket(), (SOCKADDR*)&client_address, &size_of_address) == SOCKET_ERROR)
	{
		RegisterAccept(accept_event);
		return;
	}

	session->SetAddress(NetAddress(client_address));

	char address[100];
	printf("[Listener] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		inet_ntop(AF_INET, &client_address, address, 100), ntohs(client_address.sin_port));

	session->ProcessConnect();

	RegisterAccept(accept_event);
}

void Listener::CloseSocket()
{
	closesocket(listen_socket_);
}

void Listener::Dispatch(IocpEvent* iocp_event, int num_of_bytes)
{
	ASSERT_CRASH(iocp_event->GetType() == IocpEventType::ACCEPT);
	ASSERT_CRASH(iocp_event->GetOwner() == shared_from_this());

	if (num_of_bytes != 0)
	{
		std::cout << "[Listener] Accept와 동시에 수신 : " << num_of_bytes << "bytes" << std::endl;
	}

	switch (iocp_event->GetType())
	{
	case IocpEventType::ACCEPT:
		ProcessAccept(static_cast<IocpEventAccept*>(iocp_event));
		break;
	}
}