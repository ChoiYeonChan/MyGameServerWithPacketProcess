#pragma once
#include "Service.h"

class IocpObject : public std::enable_shared_from_this<IocpObject>
{
protected:
	ServiceRef service_;

	ServiceRef GetService() const { return service_; }
	virtual void SocketIOErrorHandler(int error_code);

public:
	IocpObject(ServiceRef service) : service_(service) { }
	virtual ~IocpObject() { }

	virtual void Initialize() { service_->RegisterForIocp(this); }

	virtual void Start() abstract;
	virtual void Close() abstract;
	virtual void Dispatch(class IocpEvent* iocp_event, int num_of_bytes = 0) abstract;

	virtual HANDLE GetHandle() const abstract;
};

