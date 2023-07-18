#pragma once

#include "NetAddress.h"
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

	RecvBuffer recv_buffer_;

	std::queue<SendBufferRef> send_queue_;
	CRITICAL_SECTION lock_send_queue_;

	IocpEventRecv iocp_event_recv_;
	IocpEventSend iocp_event_send_;
	IocpEventDisconnect iocp_event_disconnect_;

	atomic<bool> is_register_send_;
	atomic<bool> is_connected_;

public:
	Session(ServiceRef service);
	virtual ~Session();

	void Start() override;
	void Close() override;

	void CreateSocket();

	// Getter & Setter
	bool IsConnected() { return is_connected_; };

	void SetAddress(NetAddress address) { address_ = address; }
	NetAddress GetAddress() const { return address_; }

	void SetSessionId(int session_id) { session_id_ = session_id; }
	int GetSessionId() const { return session_id_; }

	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

	void SetSessionSocket(SOCKET socket) { socket_ = socket; }
	SOCKET GetSessionSocket() const { return socket_; }

	virtual HANDLE GetHandle() const override { return (HANDLE)socket_; }
	virtual void Dispatch(class IocpEvent* iocp_event, int num_of_bytes = 0) override;

	// Network
private:
	void ProcessConnect();
	void RegisterDisconnect();
	void ProcessDisconnect();

	void RegisterSend();
	void ProcessSend(int num_of_bytes);

	void RegisterRecv();
	void ProcessRecv(int num_of_bytes);

protected:
	virtual void OnConnect() {};
	virtual void OnDisconnect() {};

	virtual int OnRecv(char* buffer, int length);
	virtual void OnSend(int length) {};

public:
	bool Send(SendBufferRef send_buffer);
	void Disconnect();
};

