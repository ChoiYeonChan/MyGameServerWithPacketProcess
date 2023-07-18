#pragma once

#include "NetAddress.h"
#include "SessionManager.h"
#include "ThreadManager.h"

enum class ServiceType : unsigned char
{
	SERVER,
	CLIENT
};

class Service : public std::enable_shared_from_this<Service>
{
protected:
	ServiceType type_;
	NetAddress address_;

	SessionManagerRef session_manager_;
	IocpManagerRef iocp_manager_;
	ThreadManager thread_manager_;

	std::function<SessionRef()> SessionFactory;

public:
	Service(ServiceType type, NetAddress address, std::function<SessionRef()> session_factory, int max_session_count);
	virtual ~Service() { };

	SessionRef CreateSession();
	void DestroySession(SessionRef session);

	// Network Iocp
	IocpManagerRef GetIocpManager() const { return iocp_manager_; }
	void RegisterForIocp(IocpObject* iocp_object);

	// Getter & Setter
	NetAddress GetNetAddress() const { return address_; }

	// Interface
	virtual bool Start() abstract;
	virtual void Close() abstract;
};

/*-------- ServerService --------*/

class ServerService : public Service
{
private:
	ListenerRef listener_;

public:
	ServerService(NetAddress address, std::function<SessionRef()> session_factory, int max_session_count);
	virtual ~ServerService();

	virtual bool Start() override;
	virtual void Close() override;
};
