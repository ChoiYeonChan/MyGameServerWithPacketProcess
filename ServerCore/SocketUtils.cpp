#include "pch.h"
#include "SocketUtils.h"

LPFN_CONNECTEX SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX SocketUtils::AcceptEx = nullptr;

void SocketUtils::Initialize()
{
	WSADATA wsa_data;
	ASSERT_CRASH(WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0);

	SOCKET dummy_socket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummy_socket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummy_socket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummy_socket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
	Close(dummy_socket);
}

void SocketUtils::Clear()
{
	WSACleanup();
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), &bytes, NULL, NULL)
		!= SOCKET_ERROR;
}

SOCKET SocketUtils::CreateSocket()
{
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		std::cout << "[SocketUtils] 소켓 생성에 실패했습니다. (" << WSAGetLastError() << ")" << std::endl;
	}

	return socket;
}

bool SocketUtils::Bind(SOCKET socket, NetAddress netaddress)
{
	if (::bind(socket, (SOCKADDR*)(&netaddress.GetSocketAddress()), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		std::cout << "[SocketUtils] 소켓 바인드에 실패했습니다. (" << WSAGetLastError() << ")" << std::endl;
		return false;
	}

	return true;
}

bool SocketUtils::BindAnyAddress(SOCKET socket, int port)
{
	SOCKADDR_IN my_address;
	my_address.sin_family = AF_INET;
	my_address.sin_addr.s_addr = htonl(INADDR_ANY);
	my_address.sin_port = htons(port);

	if (::bind(socket, (SOCKADDR*)&my_address, sizeof(my_address)) == SOCKET_ERROR)
	{
		std::cout << "[SocketUtils] 소켓 바인드에 실패했습니다. (" << WSAGetLastError() << ")" << std::endl;
		return false;
	}

	return true;
}

bool SocketUtils::Listen(SOCKET socket, int backlog)
{
	if (listen(socket, backlog) == SOCKET_ERROR)
	{
		std::cout << "[SocketUtils] 소켓 listen에 실패했습니다. (" << WSAGetLastError() << ")" << std::endl;
		return false;
	}

	return true;
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		closesocket(socket);
	else
		CRASH("CLOSE_INVALID_SOCKET");

	socket = INVALID_SOCKET;
}

bool SocketUtils::SetLinger(SOCKET socket, USHORT onoff, USHORT linger)
{
	LINGER option;
	option.l_onoff = onoff;
	option.l_linger = linger;
	if (setsockopt(socket, SOL_SOCKET, SO_LINGER, (char*)&option, sizeof(option)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool SocketUtils::SetUpdateAcceptContext(SOCKET socket, SOCKET listen_socket)
{
	if (setsockopt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listen_socket, sizeof(listen_socket)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int size)
{
	if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int size)
{
	if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool SocketUtils::GetRecvBufferSize(SOCKET socket, int* size)
{
	int optlen = sizeof(size);
	if (getsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)size, &optlen) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool SocketUtils::GetSendBufferSize(SOCKET socket, int* size)
{
	int optlen = sizeof(size);
	if (getsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char*)size, &optlen) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}
