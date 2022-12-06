#pragma once

#include "MemoryPool.h"

class Memory
{
private:
	static const int MAX_ALLOC_SIZE = 50;

private:
	mymp::MemoryPool* pool_table_[MAX_ALLOC_SIZE + 1];

public:
	Memory();
	~Memory();

	void* Allocate(unsigned int size);
	void Release(void* ptr);

	void Display();
};

