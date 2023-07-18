#include "pch.h"
#include "MemoryStream.h"

MemoryStream::MemoryStream() :
	buffer_(nullptr), front_(0), rear_(0),
	capacity_(0), length_(0), space_(0), is_allocated_(false)
{

}

MemoryStream::MemoryStream(unsigned int buf_size) :
	buffer_(nullptr), front_(0), rear_(0),
	capacity_(buf_size), length_(0), space_(buf_size), is_allocated_(false)
{
	if (!AllocBuffer(capacity_))
	{
		CRASH("MEMORY ALLOC FAILED");
	}
}

MemoryStream::MemoryStream(char* in_buffer, unsigned int in_buf_size, unsigned int buf_size, bool copy) :
	buffer_(nullptr), front_(0), rear_(in_buf_size),
	capacity_(buf_size), length_(in_buf_size), space_(buf_size - in_buf_size), is_allocated_(false)
{
	if (in_buf_size > buf_size)
	{
		CRASH("WRONG IN BUFFER SIZE");
	}

	if (copy)
	{
		if (!AllocBuffer(capacity_))
		{
			CRASH("MEMORY ALLOC FAILED");
		}

		memcpy_s(buffer_, capacity_, in_buffer, in_buf_size);
	}
	else
	{
		buffer_ = in_buffer;
	}
}

MemoryStream::~MemoryStream()
{
	if (is_allocated_)
	{
		std::free(buffer_);
	}
}

bool MemoryStream::AllocBuffer(const unsigned int buf_size)
{
	buffer_ = static_cast<char*>(std::malloc(buf_size));
	if (buffer_ == nullptr)
	{
		return false;
	}

	capacity_ = buf_size;
	space_ = capacity_ - length_;
	is_allocated_ = true;

	return true;
}

void MemoryStream::SetBuffer(char* buffer, const unsigned int in_buf_size, const unsigned int buf_size)
{
	if (buffer_ != nullptr && is_allocated_)
	{
		std::free(buffer_);
	}

	capacity_ = buf_size;
	length_ = in_buf_size;
	space_ = buf_size - in_buf_size;
	front_ = 0;
	rear_ = in_buf_size;
	is_allocated_ = false;

	buffer_ = buffer;
}

bool MemoryStream::OnRead(const unsigned int out_byte_count)
{
	if (length_ < out_byte_count)
	{
		CRASH("WRONG MEMORY STREAM READ");
		return false;
	}

	front_ += out_byte_count;
	space_ += out_byte_count;
	length_ -= out_byte_count;

	return true;
}

bool MemoryStream::OnWrite(const unsigned int in_byte_count)
{
	if (space_ < in_byte_count)
	{
		CRASH("WRONG MEMORY STREAM WRITE");
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

char* MemoryStream::GetDataSegment()
{
	char* segment = new char[length_];
	memcpy_s(segment, length_, buffer_ + front_, length_);
	return segment;
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

/**********************
*     StreamReader
***********************/

bool StreamReader::Read(void* out_buffer, const unsigned int out_byte_count)
{
	if (Peek(out_buffer, out_byte_count))
	{
		stream_->OnRead(out_byte_count);
		return true;
	}
	else
	{
		std::cout << "[StreamReader] Read Failed" << std::endl;
		return false;
	}
}

bool StreamReader::Peek(void* out_buffer, const unsigned int out_byte_count)
{
	if (out_buffer == nullptr || stream_->GetLength() < out_byte_count)
	{
		return false;
	}

	memcpy_s(out_buffer, out_byte_count, stream_->GetBufferFront(), out_byte_count);

	return true;
}

/**********************
*     StreamWriter
***********************/

bool StreamWriter::Write(const void* in_buffer, const unsigned int in_byte_count)
{
	if (stream_->GetSpace() < in_byte_count)
	{
		std::cout << "[StreamWrite] Write Failed" << std::endl;
		return false;
	}

	memcpy_s(stream_->GetBufferRear(), in_byte_count, in_buffer, in_byte_count);
	stream_->OnWrite(in_byte_count);

	return true;
}