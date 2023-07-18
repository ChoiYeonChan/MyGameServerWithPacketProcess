#pragma once
#include "pch.h"

class IocpObject;

enum class IocpEventType : unsigned char
{
	DISCONNECT,
	CONNECT,
	ACCEPT,
	RECV,
	SEND,
};

class IocpEvent
{
protected:
	WSAOVERLAPPED overlapped_;
	IocpEventType type_;
	IocpObjectRef owner_;

public:
	IocpEvent(IocpEventType type, IocpObjectRef owner = nullptr) : type_(type), owner_(owner)
	{
		ZeroMemoryOverlapped();
	}

	void ZeroMemoryOverlapped();

	void SetType(IocpEventType type) { type_ = type; }
	IocpEventType GetType() const { return type_; }

	void SetOwner(IocpObjectRef owner) { owner_ = owner; }
	IocpObjectRef GetOwner() const { return owner_; }

	void ResetOwner() { owner_ = nullptr; }
};

/**********************
*      IocpEvents
***********************/

class IocpEventAccept : public IocpEvent
{
public:
	IocpEventAccept(IocpObjectRef owner = nullptr) : IocpEvent(IocpEventType::ACCEPT, owner), accept_session_(nullptr)
	{

	}

	SessionRef accept_session_;
};

class IocpEventRecv : public IocpEvent
{
public:
	IocpEventRecv(IocpObjectRef owner = nullptr) : IocpEvent(IocpEventType::RECV, owner)
	{

	}

	WSABUF wsabuf_;
};

class IocpEventSend : public IocpEvent
{
public:
	IocpEventSend(IocpObjectRef owner = nullptr) : IocpEvent(IocpEventType::SEND, owner)
	{

	}

	std::vector<WSABUF> wsabufs_;
	list<SendBufferRef> send_buffer_list_;
};

class IocpEventDisconnect : public IocpEvent
{
public:
	IocpEventDisconnect(IocpObjectRef owner = nullptr) : IocpEvent(IocpEventType::DISCONNECT, owner)
	{

	}
};