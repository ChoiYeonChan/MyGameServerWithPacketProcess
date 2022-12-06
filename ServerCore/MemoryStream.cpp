#include "pch.h"
#include "MemoryStream.h"

MemoryStream::MemoryStream() :
	buffer_(nullptr), capacity_(DEFAULT_BUF_SIZE),
	front_(0), rear_(0),
	length_(0), space_(DEFAULT_BUF_SIZE)
{
	ReallocBuffer(capacity_);
}

MemoryStream::MemoryStream(unsigned int size) :
	buffer_(nullptr), capacity_(size),
	front_(0), rear_(0),
	length_(0), space_(size)
{
	ReallocBuffer(capacity_);
}

MemoryStream::MemoryStream(char* in_buffer, unsigned int in_buf_size, unsigned int size) :
	buffer_(nullptr), capacity_(size),
	front_(0), rear_(in_buf_size),
	length_(in_buf_size), space_(size - in_buf_size)
{
	if (in_buf_size > size)
	{
		CRASH("WRONG BUFFER SIZE");
	}

	ReallocBuffer(capacity_);
	memcpy_s(buffer_, capacity_, in_buffer, in_buf_size);
}

MemoryStream::~MemoryStream()
{
	std::free(buffer_);
}

bool MemoryStream::ReallocBuffer(unsigned int new_capacity)
{
	buffer_ = static_cast<char*>(std::realloc(buffer_, new_capacity));
	if (buffer_ == nullptr)
	{
		CRASH("MEMORY ALLOC FAILED");
	}

	capacity_ = new_capacity;
	space_ = capacity_ - length_;

	return true;
}

bool MemoryStream::Read(void* out_data, const unsigned int out_byte_count)
{
	if (Peek(out_data, out_byte_count))
	{
		OnRead(out_byte_count);

		return true;
	}
	else
	{
		return false;
	}
}

bool MemoryStream::Peek(void* out_data, const unsigned int out_byte_count)
{
	if (out_data == nullptr || length_ < out_byte_count)
	{
		if (out_data == nullptr)
		{
			cout << "1";
		}
		if (length_ < out_byte_count)
		{
			cout << "2 : ";
			cout << length_ << ", " << out_byte_count;
		}
		return false;
	}

	memcpy_s(out_data, out_byte_count, buffer_ + front_, out_byte_count);

	return true;
}

bool MemoryStream::OnRead(const unsigned int out_byte_count)
{
	if (length_ < out_byte_count)
	{
		return false;
	}

	front_ += out_byte_count;
	space_ += out_byte_count;
	length_ -= out_byte_count;

	return true;
}

bool MemoryStream::Write(const void* in_data, const unsigned int in_byte_count)
{
	if (space_ < in_byte_count)
	{
		return false;
	}
	
	memcpy_s(buffer_ + rear_, in_byte_count, in_data, in_byte_count);

	rear_ += in_byte_count;
	space_ -= in_byte_count;
	length_ += in_byte_count;

	return true;
}

bool MemoryStream::OnWrite(const unsigned int in_byte_count)
{
	if (space_ < in_byte_count)
	{
		return false;
	}

	rear_ += in_byte_count;
	space_ -= in_byte_count;
	length_ += in_byte_count;

	return true;
}

void MemoryStream::CleanUp()
{
	if (length_ == 0)
	{
		front_ = rear_ = 0;
	}
	else if (length_ == capacity_)
	{
		return;
	}
	else
	{
		memcpy_s(buffer_, length_, buffer_ + front_, length_);
		front_ = 0;
		rear_ = length_;
	}
}

void MemoryStream::Display()
{
	std::cout << "capacity : " << capacity_ << "\n" <<
		"front : " << front_ << "\n" <<
		"rear : " << rear_ << "\n" <<
		"length : " << length_ << "\n" <<
		"space : " << space_ << "\n";

	for (int i = 0; i < capacity_; i++)
	{
		if (i % 10 == 0)
		{
			std::cout << "\n";
		}
		printf("%d ", buffer_[i]);
	}

	std::cout << "\n";
}