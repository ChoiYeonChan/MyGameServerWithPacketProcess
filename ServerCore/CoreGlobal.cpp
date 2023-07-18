#include "pch.h"
#include "CoreGlobal.h"

Memory* g_memory;
BufferManager* g_buffer_manager;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		g_memory = new Memory();
		g_buffer_manager = new BufferManager();
	}

	~CoreGlobal()
	{
		delete g_memory;
		delete g_buffer_manager;
	}
} g_core_global;