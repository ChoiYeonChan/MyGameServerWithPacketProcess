#include "pch.h"
#include "CoreTLS.h"

thread_local uint32			L_thread_id = 0;
thread_local BufferChunkRef	L_buffer_chunk = nullptr;