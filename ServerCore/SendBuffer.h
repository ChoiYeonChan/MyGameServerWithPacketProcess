#pragma once

#include "MemoryStream.h"

class SendBuffer : public MemoryStream
{
public:
	SendBuffer(int size) : MemoryStream(size)
	{

	}
};

