#include "pch.h"
#include "NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN address) : address_(address)
{
}

NetAddress::NetAddress(wstring ip, int port)
{
	ZeroMemory(&address_, sizeof(address_));
	address_.sin_family = AF_INET;
	address_.sin_addr = Ip2Address(ip.c_str());
	address_.sin_port = htons(port);
}

wstring NetAddress::GetIPAddress()
{
	WCHAR buffer[100];
	InetNtopW(AF_INET, &address_.sin_addr, buffer, len32(buffer));
	return wstring(buffer);
}

IN_ADDR NetAddress::Ip2Address(const WCHAR* ip)
{
	IN_ADDR address;
	InetPtonW(AF_INET, ip, &address);
	return address;
}
