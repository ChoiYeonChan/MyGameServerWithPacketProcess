#include "pch.h"
#include "Listener.h"
#include "Session.h"

bool Listener::StartAccept()
{
	listen_socket_ = SocketUtils::CreateSocket();
	service_->RegisterForIocp(this);

	if (SocketUtils::SetLinger(listen_socket_, 0, 0) == false)
		return false;

	if (SocketUtils::SetReuseAddress(listen_socket_, true) == false)
		return false;

	SocketUtils::Bind(listen_socket_, service_->GetNetAddress());
	SocketUtils::Listen(listen_socket_);

	int accept_count = 50;
	for (int i = 0; i < accept_count; i++)
	{
		IocpEventAccept* accept_event = new IocpEventAccept(shared_from_this());
		accept_event_list_.push_back(accept_event);
		RegisterAccept(accept_event);
	}

	return true;
}

void Listener::RegisterAccept(IocpEventAccept* accept_event)
{
	SessionRef session = service_->CreateSession();
	accept_event->ZeroMemoryOverlapped();
	accept_event->SetSessionAccepted(session);

	DWORD bytes_received;

	if (SocketUtils::AcceptEx(listen_socket_, session->GetSessionSocket(), session->recv_buffer_.GetBufferFront(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes_received, (LPOVERLAPPED)(accept_event)))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			RegisterAccept(accept_event);
		}
		else
		{
			std::cout << "Listener RegisterAccept Error (" << WSAGetLastError() << ")" << std::endl;
		}
	}
}

void Listener::ProcessAccept(IocpEventAccept* accept_event)
{
	SessionRef session = accept_event->GetSessionAccepted();

	// Accept와 달리 AcceptEx는 클라이언트 소켓을 미리 생성해둔 상태이므로 ListenSocket의
	// 속성을 상속하지 않는다. 따라서 별도로 속성을 상속하도록 지정해주어야 한다.
	if (SocketUtils::SetUpdateAcceptContext(session->GetSessionSocket(), listen_socket_) == false)
	{
		RegisterAccept(accept_event);
		std::cout << "SetSetUpdateAcceptContext failed.." << WSAGetLastError() << std::endl;
		return;
	}

	SOCKADDR_IN client_address;
	int size_of_address = sizeof(client_address);
	if (getpeername(session->GetSessionSocket(), (SOCKADDR*)&client_address, &size_of_address) == SOCKET_ERROR)
	{
		RegisterAccept(accept_event);
		std::cout << "getpeername failed.." << WSAGetLastError() << std::endl;
		return;
	}

	session->SetAddress(NetAddress(client_address));

	char address[100];
	printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
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
		std::cout << "recv bytes with accept : " << num_of_bytes << endl;
	}

	switch (iocp_event->GetType())
	{
	case IocpEventType::ACCEPT:
		ProcessAccept(static_cast<IocpEventAccept*>(iocp_event));
		break;
	}
}