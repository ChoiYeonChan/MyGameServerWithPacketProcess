#pragma once

#include "NetAddress.h"
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

	std::list<SessionRef> session_manager_;
	CRITICAL_SECTION lock_session_list_;

	int current_session_count_;
	int max_session_count_;

	int current_session_id_;

	IocpManagerRef iocp_manager_;
	ThreadManager thread_manager_;

public:
	Service(ServiceType type, NetAddress address, int max_session_count);
	virtual ~Service() {};

	// Network Session
	SessionRef CreateSession();
	void DestroySession(SessionRef session);

	int GetCurrentSessionCount() const { return current_session_count_; }
	int GetMaxSessionCount() const { return max_session_count_; }

	void DisplaySessionList();	// temp

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
	ListenerRef Listener_;

public:
	ServerService(NetAddress address, int max_session_count);
	virtual ~ServerService();

	virtual bool Start() override;
	virtual void Close() override;
};

