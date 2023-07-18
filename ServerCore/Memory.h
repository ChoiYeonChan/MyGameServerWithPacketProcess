#pragma once

#include <map>
#include <vector>
#include "MemoryPool.h"

class Memory
{
private:
	enum
	{
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

	std::map<int, int> map_index_;
	std::vector<mymp::MemoryPool*> pool_list_;
	mymp::MemoryPool* pool_table_[MAX_ALLOC_SIZE + 1];

public:
	Memory();
	~Memory();

	void* Allocate(unsigned int size);
	void Release(void* ptr);

	void Display();
};

