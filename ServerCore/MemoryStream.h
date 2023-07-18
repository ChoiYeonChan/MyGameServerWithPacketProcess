#pragma once

class MemoryStream
{
protected:
	static const int DEFAULT_BUF_SIZE = 1024;

protected:
	char* buffer_;
	unsigned int front_;
	unsigned int rear_;
	unsigned int capacity_;
	unsigned int length_;
	unsigned int space_;

	bool is_allocated_;

public:
	MemoryStream();
	MemoryStream(unsigned int buf_size);
	MemoryStream(char* in_buffer, unsigned int in_buf_size, unsigned int buf_size = DEFAULT_BUF_SIZE, bool copy = false);

	~MemoryStream();

	void SetBuffer(char* buffer, const unsigned int in_buf_size, const unsigned int buf_size);

	// Read & Write
	bool OnRead(const unsigned int out_byte_count);
	bool OnWrite(const unsigned int in_byte_count);

	void CleanUp();

	// Getter & Setter
	char* GetBufferFront() const { return buffer_ + front_; }
	char* GetBufferRear() const { return buffer_ + rear_; }
	unsigned int GetCapacity() const { return capacity_; }
	unsigned int GetLength() const { return length_; }
	unsigned int GetSpace() const { return space_; }

	char* GetDataSegment();
	void Display();

private:
	bool AllocBuffer(const unsigned int buf_size);
};

class StreamReader
{
private:
	MemoryStream* stream_;

public:
	StreamReader(MemoryStream* stream) : stream_(stream) { }

	template <typename T>
	bool Read(T* dest) { return Read(dest, sizeof(T)); }
	bool Read(void* out_buffer, const unsigned int out_byte_count);
	bool Peek(void* out_buffer, const unsigned int out_byte_count);

	template <typename T>
	StreamReader& operator>>(T& dest);
};

template <typename T>
StreamReader& StreamReader::operator>>(T& dest)
{
	Read<T>(&dest);
	return *this;
}

class StreamWriter
{
private:
	MemoryStream* stream_;

public:
	StreamWriter(MemoryStream* stream) : stream_(stream) { }

	template <typename T>
	bool Write(const T* source) { return Write(source, sizeof(T)); }
	bool Write(const void* in_buffer, const unsigned int in_byte_count);

	template <typename T>
	T* Reserve(int count);

	template <typename T>
	StreamWriter& operator<<(T&& source);
};

template <typename T>
T* StreamWriter::Reserve(int count)
{
	T* ptr = reinterpret_cast<T*>(stream_->GetBufferRear());
	if (stream_->OnWrite(sizeof(T) * count))
	{
		return ptr;
	}
	else
	{
		return nullptr;
	}
}

template <typename T>
StreamWriter& StreamWriter::operator<<(T&& source)
{
	Write<T>(&source);
	return *this;
}