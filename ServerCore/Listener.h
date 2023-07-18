#pragma once

#include "IocpEvent.h"
#include "IocpObject.h"

#include "NetAddress.h"

class IocpEvent;

class Listener : public IocpObject
{
private:
	SOCKET listen_socket_;

	std::vector<IocpEventAccept*> accept_event_list_;

public:
	Listener(ServiceRef service) : IocpObject(service), listen_socket_(INVALID_SOCKET) { };
	virtual ~Listener() { };

	void Start() override;
	void Close() override;

	bool StartAccept();

	SOCKET GetListenSocket() const { return listen_socket_; }
	void CloseSocket();

	virtual HANDLE GetHandle() const override { return (HANDLE)listen_socket_; }
	virtual void Dispatch(IocpEvent* iocp_event, int num_of_bytes = 0) override;

private:
	void RegisterAccept(IocpEventAccept* accept_event);
	void ProcessAccept(IocpEventAccept* accept_event);
};