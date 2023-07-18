#pragma once
#include <array>
#include "pch.h"
#include "MemoryStream.h"

class SendBuffer : public MemoryStream
{
private:
	BufferChunkRef owner_ = nullptr;

public:
	SendBuffer() = default;
	SendBuffer(int size) : MemoryStream(size) { }
	SendBuffer(char* in_buffer, unsigned int in_buf_size, unsigned int buf_size) : MemoryStream(in_buffer, in_buf_size, buf_size, true) { }
	~SendBuffer() = default;

	void Open(int size);
	void Close();
};

class BufferChunk : public enable_shared_from_this<BufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 819200
	};

private:
	std::array<char, SEND_BUFFER_CHUNK_SIZE> buffer_ = {};
	bool is_open_ = false;
	int used_size_ = 0;

public:
	BufferChunk() = default;
	~BufferChunk() = default;

	void Reset();
	char* Open(int alloc_size);
	void Close(int write_size);

	char* Buffer() { return &buffer_[used_size_]; }
	int FreeSize() { return buffer_.size() - used_size_; }
	bool IsOpen() { return is_open_; }
};

class BufferManager
{
private:
	CRITICAL_SECTION lock_;
	std::vector<BufferChunkRef> buffer_chunk_list_;

public:
	BufferManager();

	BufferChunkRef Pop();
	void Push(BufferChunkRef buffer);

	static void PushGlobal(BufferChunk* buffer);
};