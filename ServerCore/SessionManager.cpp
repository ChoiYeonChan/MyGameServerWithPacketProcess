#include "pch.h"
#include "SessionManager.h"

SessionManager::SessionManager(int max_session_count) :
	max_session_count_(max_session_count), current_session_count_(0)
{
	id_map_ = new std::atomic<bool>[max_session_count_];
	InitializeCriticalSection(&lock_session_list_);
}

SessionManager::~SessionManager() { }

bool SessionManager::GetSessionId(SessionRef session)
{
	int traverse_count = 0;
	int max_traverse_count = 5;

	while (true)
	{
		for (int index = 0; index < max_session_count_; index++)
		{
			bool expected = false;
			if (id_map_[index].compare_exchange_strong(expected, true))
			{
				session->SetSessionId(index);
				return true;
			}
		}

		if (++traverse_count == max_traverse_count)
		{
			std::cout << "[SessionManager] 모든 세션 ID가 사용 중입니다." << std::endl;
			break;
		}
	}

	return false;
}

bool SessionManager::ReturnSessionId(int session_id)
{
	bool expected = true;
	if (id_map_[session_id].compare_exchange_strong(expected, false))
	{
		return true;
	}
	else
	{
		CRASH("SESSION ID MAP CORRUPTED");
		return false;
	}
}

SessionRef SessionManager::FindSession(int session_id)
{
	SessionRef session = nullptr;

	{
		EnterCriticalSection(&lock_session_list_);
		if (session_list_.find(session_id) != session_list_.end())
		{
			session = session_list_[session_id];
		}
		LeaveCriticalSection(&lock_session_list_);
	}

	return session;
}

int SessionManager::AddSession(SessionRef session)
{
	if (GetSessionId(session))
	{
		{
			EnterCriticalSection(&lock_session_list_);
			if (session_list_.find(session->GetSessionId()) != session_list_.end())
			{
				CRASH("SESSION ALREADY EXIST");
			}
			else
			{
				session_list_.insert({ session->GetSessionId(), session });
			}
			LeaveCriticalSection(&lock_session_list_);
		}
		return session->GetSessionId();
	}
	else
	{
		std::cout << "[SessionManager] 세션 아이디를 설정하는데 실패했습니다." << std::endl;
		return -1;
	}
}

void SessionManager::RemoveSession(int session_id)
{
	{
		EnterCriticalSection(&lock_session_list_);
		if (session_list_.find(session_id) != session_list_.end())
		{
			SessionRef session = session_list_[session_id];
			session_list_.erase(session_id);
		}
		LeaveCriticalSection(&lock_session_list_);
	}

	ReturnSessionId(session_id);
}

void SessionManager::DisplaySessionList()
{
	std::cout << "[SessionManager] 세션 ID 리스트" << std::endl;
	{
		EnterCriticalSection(&lock_session_list_);
		for (auto it = session_list_.begin(); it != session_list_.end(); it++)
		{
			std::cout << it->first << " ";
		}
		LeaveCriticalSection(&lock_session_list_);
	}
	std::cout << std::endl;
}