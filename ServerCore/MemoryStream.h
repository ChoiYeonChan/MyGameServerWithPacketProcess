#pragma once

class MemoryStream
{
private:
	static const int DEFAULT_BUF_SIZE = 1024;

private:
	char* buffer_;
	unsigned int front_;
	unsigned int rear_;
	unsigned int capacity_;
	unsigned int length_;
	unsigned int space_;

	bool ReallocBuffer(unsigned int new_capacity);

public:
	MemoryStream();
	MemoryStream(unsigned int buf_size);
	MemoryStream(char* in_buffer, unsigned int in_buf_size, unsigned int size = DEFAULT_BUF_SIZE);

	~MemoryStream();

	bool Read(void* out_data, const unsigned int out_byte_count);
	bool Peek(void* out_data, const unsigned int out_byte_count);
	bool OnRead(const unsigned int out_byte_count);

	bool Write(const void* in_data, const unsigned int in_byte_count);
	bool OnWrite(const unsigned int int_byte_count);

	void CleanUp();

	char* GetBufferFront() const { return buffer_ + front_; }
	char* GetBufferRear() const { return buffer_ + rear_; }
	unsigned int GetCapacity() const { return capacity_; }
	unsigned int GetLength() const { return length_; }
	unsigned int GetSpace() const { return space_; }

	void Display();		// temp
};

