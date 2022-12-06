#include "pch.h"

#include "CoreGlobal.h"
#include "Memory.h"
#include "ThreadManager.h"

ThreadManager* g_thread_manager = nullptr;
Memory* g_memory = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		g_thread_manager = new ThreadManager();
		g_memory = new Memory();
	}

	~CoreGlobal()
	{
		delete g_thread_manager;
		delete g_memory;
	}
} g_core_global;