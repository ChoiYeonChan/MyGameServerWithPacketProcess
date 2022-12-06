#include "pch.h"
#include "IocpEvent.h"

void IocpEvent::ZeroMemoryOverlapped()
{
	ZeroMemory(&overlapped_, sizeof(WSAOVERLAPPED));
}
