#include "pch.h"
#include "ThreadManager.h"

ThreadManager::ThreadManager()
{
	InitializeTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
	DestroyTLS();
}

void ThreadManager::Push(std::function<void(void)> callback)
{
	{
		std::lock_guard<std::mutex> lock(lock_thread_list_);
		thread_list_.push_back(std::thread([=]()
			{
				InitializeTLS();
				callback();
				DestroyTLS();
			}
		));
	}
}

void ThreadManager::Join()
{
	for (auto& thread : thread_list_)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	thread_list_.clear();
}

void ThreadManager::InitializeTLS()
{
	static std::atomic<unsigned int> thread_id = 0;
	L_thread_id = thread_id.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
}
