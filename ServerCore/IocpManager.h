#pragma once

class IocpObject;

class IocpManager
{
private:
	HANDLE iocp_handle_;

public:
	IocpManager();
	~IocpManager();

	HANDLE GetIocpHandle() const { return iocp_handle_; }

	bool Register(IocpObject* object);
	bool WorkerThreadFunction(int timeout = INFINITE);
};

