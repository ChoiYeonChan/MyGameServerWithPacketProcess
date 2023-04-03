#include "pch.h"
#include "Service.h"
#include "IocpManager.h"
#include "Listener.h"
#include "Session.h"


Service::Service(ServiceType type, NetAddress address, int max_session_count)
    : type_(type), address_(address), max_session_count_(max_session_count), 
    current_session_count_(0), current_session_id_(0)
{
    iocp_manager_ = make_shared<IocpManager>();
    InitializeCriticalSection(&lock_session_list_);
}

SessionRef Service::CreateSession()
{
    SessionRef session = make_shared<Session>(shared_from_this());

    {
        EnterCriticalSection(&lock_session_list_);

        session->SetSessionId(current_session_id_++);

        session_manager_.push_back(session);
        ++current_session_count_;
        
        LeaveCriticalSection(&lock_session_list_);
    }

    return session;
}

void Service::DestroySession(SessionRef session)
{
    {
        EnterCriticalSection(&lock_session_list_);
        session_manager_.remove(session);
        --current_session_count_;

        session->SetSessionId(-1);  // temp

        LeaveCriticalSection(&lock_session_list_);
    }
}

void Service::DisplaySessionList()
{
    for (auto it : session_manager_)
    {
        cout << it->GetSessionId() << " ";
    }
}

void Service::RegisterForIocp(IocpObject* iocp_object)
{
    iocp_manager_->Register(iocp_object);
}

ServerService::ServerService(NetAddress address, int max_session_count)
    : Service(ServiceType::SERVER, address, max_session_count)
{
    SocketUtils::Initialize();
}

ServerService::~ServerService()
{
    SocketUtils::Clear();
}

bool ServerService::Start()
{
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);

    for (int i = 0; i < system_info.dwNumberOfProcessors * 2; i++)
    {
        thread_manager_.Push([=]() { iocp_manager_->WorkerThreadFunction(); });
    }

    listener_ = make_shared<Listener>(static_pointer_cast<Service>(shared_from_this()));
    listener_->StartAccept();

    return true;
}

void ServerService::Close()
{
    thread_manager_.Join();
}