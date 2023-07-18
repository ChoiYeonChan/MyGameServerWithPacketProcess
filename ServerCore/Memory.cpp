#include "pch.h"
#include "Memory.h"

Memory::Memory()
{
	int size = 0;
	int table_index = 0;

	for (size = 32; size < 1024; size += 32)
	{
		mymp::MemoryPool* pool = new mymp::MemoryPool(size);
		pool_list_.push_back(pool);

		while (table_index <= size)
		{
			map_index_[table_index] = size;
			pool_table_[table_index] = pool;
			table_index++;
		}
	}

	for (; size < 2048; size += 128)
	{
		mymp::MemoryPool* pool = new mymp::MemoryPool(size);
		pool_list_.push_back(pool);

		while (table_index <= size)
		{
			map_index_[table_index] = size;
			pool_table_[table_index] = pool;
			table_index++;
		}
	}

	for (; size <= 4096; size += 256)
	{
		mymp::MemoryPool* pool = new mymp::MemoryPool(size);
		pool_list_.push_back(pool);

		while (table_index <= size)
		{
			map_index_[table_index] = size;
			pool_table_[table_index] = pool;
			table_index++;
		}
	}

	/*
	for (int i = 0; i <= 4096; i++)
	{
		if (map_index_[i] != pool_table_[i]->AllocSize())
		{
			std::cout << "memory() error!" << std::endl;
			std::cout << i << " " << map_index_[i] << " " << pool_table_[i] << std::endl;
		}
	}
	*/
}

Memory::~Memory()
{
	for (mymp::MemoryPool* pool : pool_list_)
		delete pool;

	pool_list_.clear();
}

void* Memory::Allocate(unsigned int size)
{
	if (size <= 0)
	{
		std::cout << "잘못된 할당 요청입니다." << std::endl;
		return nullptr;
	}

	mymp::BlockHeader* header = nullptr;
	int alloc_size = size + sizeof(mymp::BlockHeader);

	if (size > MAX_ALLOC_SIZE)
	{
		header = (mymp::BlockHeader*)_aligned_malloc(size + sizeof(mymp::BlockHeader), 16);
	}
	else
	{
		pool_table_[size]->Pop(header);
	}

	if (header == nullptr)
	{
		std::cout << "메모리 할당에 실패했습니다. (header)" << std::endl;
		return nullptr;
	}

	return mymp::BlockHeader::AttachHeader(header, map_index_[size]);
}

void Memory::Release(void* ptr)
{
	mymp::BlockHeader* header = mymp::BlockHeader::DetachHeader(ptr);

	short alloc_size = header->size;
	if (alloc_size <= 0)
	{
		std::cout << "잘못된 메모리 해제입니다." << alloc_size << std::endl;
		// CRASH
		return;
	}

	if (alloc_size > MAX_ALLOC_SIZE)
	{
		_aligned_free(header);
	}
	else
	{
		pool_table_[alloc_size]->Push(header);
	}
}

void Memory::Display()
{
	for (int i = 0; i < 100; i++)
	{
		std::cout << '[' << i << "] Alloc : " << pool_table_[i]->AllocCount()
			<< ", Release : " << pool_table_[i]->ReleaseCount() << std::endl;
	}
}