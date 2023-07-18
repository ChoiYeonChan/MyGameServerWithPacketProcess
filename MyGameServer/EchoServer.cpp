#include "pch.h"

#include "Service.h"
#include "ObjectPool.h"

#define PROCADEMY_SERVER_PORT 6000

ServerServiceRef service;
SessionRef func()
{
	return MakeShared<Session>(service);
}

int main(void)
{
	service = make_shared<ServerService>(NetAddress(L"127.0.0.1", PROCADEMY_SERVER_PORT), func, 5000);
	service->Start();
	while (true)
	{
		;
	}


	return 0;
}