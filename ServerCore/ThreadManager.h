#pragma once

extern thread_local unsigned int L_thread_id;

class ThreadManager
{
private:
	std::vector<std::thread> thread_list_;
	std::mutex lock_thread_list_;

public:
	ThreadManager();
	~ThreadManager();

	void Push(std::function<void(void)> callback);
	void Join();

	void InitializeTLS();
	void DestroyTLS();
};

