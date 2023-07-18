#pragma once
#include "Session.h"

class SessionManager
{
private:
	std::map<int, SessionRef> session_list_;
	CRITICAL_SECTION lock_session_list_;

	int current_session_count_;
	int max_session_count_;

	std::atomic<bool>* id_map_;

public:
	SessionManager(int max_session_count);
	~SessionManager();

	SessionRef FindSession(int session_id);
	int AddSession(SessionRef session);
	void RemoveSession(int session_id);

	void DisplaySessionList();

private:
	bool GetSessionId(SessionRef session);
	bool ReturnSessionId(int session_id);
};