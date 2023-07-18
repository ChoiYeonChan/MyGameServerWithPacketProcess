#pragma once
#include "MemoryStream.h"

class RecvBuffer : public MemoryStream
{
public:
	RecvBuffer(int size) : MemoryStream(size)
	{

	}
};

