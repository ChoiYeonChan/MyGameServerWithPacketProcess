#include "pch.h"
#include "SendBuffer.h"

void SendBuffer::Open(int size)
{
	if (L_buffer_chunk == nullptr)
	{
		L_buffer_chunk = g_buffer_manager->Pop();
		L_buffer_chunk->Reset();
	}

	if (L_buffer_chunk->FreeSize() < size)
	{
		L_buffer_chunk = g_buffer_manager->Pop();
		L_buffer_chunk->Reset();
	}

	owner_ = L_buffer_chunk;
	MemoryStream::SetBuffer(L_buffer_chunk->Open(size), 0, size);
}

void SendBuffer::Close()
{
	ASSERT_CRASH(capacity_ >= rear_);
	owner_->Close(rear_);
}

void BufferChunk::Reset()
{
	is_open_ = false;
	used_size_ = 0;
}

char* BufferChunk::Open(int alloc_size)
{
	// std::cout << "used_size : " << used_size_ << std::endl;

	if (is_open_ == true)
	{
		// std::cout << "Áßº¹ ¿ÀÇÂ" << std::endl;
	}

	if (alloc_size > FreeSize())
	{
		return nullptr;
	}

	is_open_ = true;
	return Buffer();
	// return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), alloc_size);
}

void BufferChunk::Close(int write_size)
{
	is_open_ = false;
	used_size_ += write_size;
}

BufferManager::BufferManager()
{
	InitializeCriticalSection(&lock_);
}

BufferChunkRef BufferManager::Pop()
{
	{
		EnterCriticalSection(&lock_);
		// std::cout << "enter cri\n";
		if (buffer_chunk_list_.empty() == false)
		{
			BufferChunkRef buffer_chunk = buffer_chunk_list_.back();
			buffer_chunk_list_.pop_back();
			LeaveCriticalSection(&lock_);
			return buffer_chunk;
		}
		LeaveCriticalSection(&lock_);
		// std::cout << "leave cri\n";
	}

	return BufferChunkRef(xnew<BufferChunk>(), PushGlobal);
}

void BufferManager::Push(BufferChunkRef buffer)
{
	{
		EnterCriticalSection(&lock_);
		buffer_chunk_list_.push_back(buffer);
		LeaveCriticalSection(&lock_);
	}
}

void BufferManager::PushGlobal(BufferChunk* buffer)
{
	// std::cout << "PushGlobal" << std::endl;
	g_buffer_manager->Push(BufferChunkRef(buffer, PushGlobal));
}