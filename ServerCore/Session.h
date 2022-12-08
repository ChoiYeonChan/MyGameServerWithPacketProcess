#pragma once

#include "IocpEvent.h"
#include "IocpObject.h"
#include "RecvBuffer.h"

class Session : public IocpObject
{
	friend class Listener;

private:
	SOCKET socket_;
	NetAddress address_;

	int session_id_;

	std::queue<SendBufferRef> send_queue_;
	CRITICAL_SECTION lock_send_queue_;

	RecvBuffer recv_buffer_;

	IocpEventRecv iocp_event_recv_;
	IocpEventSend iocp_event_send_;
	IocpEventDisconnect iocp_event_disconnect_;

	atomic<bool> is_register_send_;
	atomic<bool> is_connected_;

public:
	Session(ServiceRef service);
	virtual ~Session();

	void SetAddress(NetAddress address) { address_ = address; }
	NetAddress GetAddress() const { return address_; }

	void SetSessionId(int session_id) { session_id_ = session_id; }
	int GetSessionId() const { return session_id_; }

	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

	SOCKET GetSessionSocket() const { return socket_; }

	virtual HANDLE GetHandle() const override { return (HANDLE)socket_; }
	virtual void Dispatch(class IocpEvent* iocp_event, int num_of_bytes = 0) override;

	// NetWork Send
private:
	void RegisterSend();
	void ProcessSend(int num_of_bytes);

	void RegisterRecv();
	void ProcessRecv(int num_of_bytes);

	void RegisterDisconnect();
	void ProcessDisconnect();
	void ProcessConnect();

protected:
	virtual void OnSend() {};
	virtual int OnRecv(char* buffer, int length);
	virtual void OnDisconnect() {};

public:
	bool Send(SendBufferRef send_buffer);
	void Disconnect();
};


struct PacketHeader
{
	unsigned short id;
	unsigned short size;
};

class PacketSession : public Session
{
public:
	PacketSession(ServiceRef service);
	virtual ~PacketSession();

	PacketSessionRef GetPacketSessionRef() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int OnRecv(char* buffer, int length) override;
	virtual int OnRecvPacket(char* buffer, int length) abstract;
};
