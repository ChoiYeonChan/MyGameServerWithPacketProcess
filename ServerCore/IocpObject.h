#pragma once

#include "Service.h"

class IocpObject : public std::enable_shared_from_this<IocpObject>
{
protected:
	ServiceRef service_;

	ServiceRef GetServiceRef() const { return service_; }

public:
	IocpObject(ServiceRef service) : service_(service)
	{

	}

	virtual ~IocpObject() {};

	virtual HANDLE GetHandle() const abstract;
	virtual void Dispatch(class IocpEvent* iocp_event, int num_of_bytes = 0) abstract;
};