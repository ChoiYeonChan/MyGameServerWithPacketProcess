#include "pch.h"
#include "IocpManager.h"
#include "IocpEvent.h"
#include "IocpObject.h"

IocpManager::IocpManager()
{
	iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	ASSERT_CRASH(iocp_handle_ != INVALID_HANDLE_VALUE);
}

IocpManager::~IocpManager()
{
	CloseHandle(iocp_handle_);
}

bool IocpManager::Register(IocpObject* object)
{
	return CreateIoCompletionPort(object->GetHandle(), iocp_handle_, (ULONG_PTR)object, 0);
}

bool IocpManager::WorkerThreadFunction(int timeout)
{
	while (true)
	{
		DWORD num_of_bytes = 0;
		IocpObject* iocp_object = nullptr;
		IocpEvent* iocp_event = nullptr;

		if (GetQueuedCompletionStatus(iocp_handle_, &num_of_bytes, (PULONG_PTR)&iocp_object, (LPOVERLAPPED*)&iocp_event, timeout)
			== TRUE)
		{
			iocp_object->Dispatch(iocp_event, num_of_bytes);
		}
		else
		{
			std::cout << "GQCS Failed, error code : " << WSAGetLastError() << ", bytes : " << num_of_bytes << std::endl;
			int error_code = WSAGetLastError();
			switch (error_code)
			{
			case WAIT_TIMEOUT:
				return false;
			default:
				iocp_object->Dispatch(iocp_event, num_of_bytes);
				break;
			}
		}
	}

	return true;
}