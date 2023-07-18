#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "SessionManager.h"
#include "IocpManager.h"
#include "Listener.h"

Service::Service(ServiceType type, NetAddress address, std::function<SessionRef()> session_factory, int max_session_count)
    : type_(type), address_(address), SessionFactory(session_factory)
{
    session_manager_ = make_shared<SessionManager>(max_session_count);
    iocp_manager_ = make_shared<IocpManager>();
}

SessionRef Service::CreateSession()
{
    SessionRef session = SessionFactory();
    if (session_manager_->AddSession(session) == -1)
    {
        return nullptr;
    }

    return session;
}

void Service::DestroySession(SessionRef session)
{
    session_manager_->RemoveSession(session->GetSessionId());
}

void Service::RegisterForIocp(IocpObject* iocp_object)
{
    if (!iocp_manager_->Register(iocp_object))
    {
        std::cout << "IOCP 등록에 실패했습니다." << std::endl;
    }
}

ServerService::ServerService(NetAddress address, std::function<SessionRef()> session_factory, int max_session_count)
    : Service(ServiceType::SERVER, address, session_factory, max_session_count)
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
    listener_->Start();

    return true;
}

void ServerService::Close()
{
    thread_manager_.Join();
}