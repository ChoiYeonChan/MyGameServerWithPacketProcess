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

/*
1. overlapped 구조체를 래핑하는 역할
2. 어떤 작업을 완료했는가 (IocpEventType)
3. 해당 작업에 대한 추가 정보를 저장하는 역할
*/

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

/***************************
*	     IocpEvents
****************************/

class IocpEventAccept : public IocpEvent
{
private:
	SessionRef session_accepted_;

public:
	IocpEventAccept(IocpObjectRef owner = nullptr) : IocpEvent(IocpEventType::ACCEPT, owner), session_accepted_(nullptr)
	{

	}

	void SetSessionAccepted(SessionRef session) { session_accepted_ = session; }
	SessionRef GetSessionAccepted() const { return session_accepted_; }
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