#pragma once

class NetAddress
{
private:
	SOCKADDR_IN address_;

public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN address);
	NetAddress(wstring ip, int port);

	SOCKADDR_IN& GetSocketAddress() { return address_; }
	wstring GetIPAddress();

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);
};

