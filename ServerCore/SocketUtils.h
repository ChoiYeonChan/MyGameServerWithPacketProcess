#pragma once

#include "NetAddress.h"

class SocketUtils
{
public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_ACCEPTEX AcceptEx;

public:
	static void Initialize();
	static void Clear();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);

	// basic socket functions
	static SOCKET CreateSocket();
	static bool Bind(SOCKET socket, NetAddress netaddress);
	static bool BindAnyAddress(SOCKET socket, int port);
	static bool Listen(SOCKET socket, int backlog = SOMAXCONN);
	static void Close(SOCKET& socket);

	// socket options
	static bool SetLinger(SOCKET socket, USHORT onoff, USHORT linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetUpdateAcceptContext(SOCKET socket, SOCKET listen_socket);

	static bool SetRecvBufferSize(SOCKET socket, int size);
	static bool SetSendBufferSize(SOCKET socket, int size);

	static bool GetRecvBufferSize(SOCKET socket, int* size);
	static bool GetSendBufferSize(SOCKET socket, int* size);
};
