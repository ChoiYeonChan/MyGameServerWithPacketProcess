#include "pch.h"
#include "Memory.h"

Memory::Memory()
{
	for (int size = 0; size <= MAX_ALLOC_SIZE; size++)
	{
		pool_table_[size] = new mymp::MemoryPool(size);
	}
}

Memory::~Memory()
{
	for (int size = 0; size <= MAX_ALLOC_SIZE; size++)
	{
		delete pool_table_[size];
	}
}

void* Memory::Allocate(unsigned int size)
{
	mymp::BlockHeader* alloc_node = nullptr;
	int alloc_size = size + sizeof(mymp::BlockHeader);

#ifdef __STOMP__
	alloc_node = (mymp::BlockHeader*)(StompAllocator::Allocate(alloc_size));
#else
	// �Ҵ� ����� �ִ� ������� ũ�ٸ� �޸�Ǯ���� �������� �ʰ� �޸� �Ҵ��Ѵ�.
	if (size > MAX_ALLOC_SIZE)
	{
		alloc_node = (mymp::BlockHeader*)_aligned_malloc(size + sizeof(mymp::BlockHeader), 16);
	}
	else
	{
		pool_table_[size]->Pop(alloc_node);
	}
#endif
	if (alloc_node == nullptr)
	{
		std::cout << "[Memory] memory allocate failed" << std::endl;
		return nullptr;
	}

	return mymp::BlockHeader::AttachHeader(alloc_node, size);
}

void Memory::Release(void* ptr)
{
	mymp::BlockHeader* release_node = mymp::BlockHeader::DetachHeader(ptr);

	int alloc_size = release_node->size;
	if (alloc_size <= 0)
	{
		// CRASH
	}

#ifdef __STOMP__
	StompAllocator::Release(release_node);
#else
	// �ִ� ������� Ŭ ���� �޸� Ǯ�� �ݳ����� �ʰ� �޸� �����Ѵ�.	
	if (alloc_size > MAX_ALLOC_SIZE)
	{
		_aligned_free(release_node);
	}
	else
	{
		pool_table_[alloc_size]->Push(release_node);
	}
#endif
}

void Memory::Display()
{
	for (int i = 0; i < MAX_ALLOC_SIZE; i++)
	{
		std::cout << '[' << i << "] Alloc : " << pool_table_[i]->AllocCount()
		<< ", Release : " << pool_table_[i]->ReleaseCount() << std::endl;		
	}
}