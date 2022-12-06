#include "pch.h"
#include "ThreadManager.h"
#include <iostream>

thread_local unsigned int L_thread_id = -1;

ThreadManager::ThreadManager()
{
	InitializeTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Push(std::function<void(void)> callback)
{
	std::lock_guard<std::mutex> lock(list_lock_);
	thread_list_.push_back(std::thread([=]()
		{
			InitializeTLS();
			callback();
			DestroyTLS();
		})
	);
}

void ThreadManager::Join()
{
	for (auto& t : thread_list_)
	{	
		//if (t.joinable())
		{
			t.join();
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