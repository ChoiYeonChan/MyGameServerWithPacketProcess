#include "pch.h"

#include "Service.h"
#include "ObjectPool.h"

#define SERVER_PORT 6000

int main(void)
{
	ServerServiceRef service = make_shared<ServerService>(NetAddress(L"127.0.0.1", SERVER_PORT), 5000);
	service->Start();
	service->Close();

	return 0;
}