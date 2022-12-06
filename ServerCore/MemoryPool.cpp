#include "pch.h"
#include "MemoryPool.h"

/*-------------------
	LockFreeStack
-------------------*/

mymp::LockFreeStack::LockFreeStack() : size(0)
{
	top = (StampNode*)_aligned_malloc(sizeof(StampNode), 16);
	if (top == nullptr)
	{
		std::cout << "LockFreeStack() alloc memory falid" << std::endl;
		// CRASH("MEMORY ALLOCATE FAILED");
	}
	new(top)StampNode;
}

mymp::LockFreeStack::~LockFreeStack()
{
	Clear();
}

void mymp::LockFreeStack::Clear()
{
	BlockHeader* ptr = nullptr;
	
	while (top->ptr != nullptr)
	{
		ptr = top->ptr;
		top->ptr = top->ptr->next;
		_aligned_free(ptr);
	}
	
	InterlockedExchange64(&top->stamp, 0);
	size = 0;
}

void mymp::LockFreeStack::Push(BlockHeader* node)
{
	while (true)
	{
		StampNode oldTop;
		oldTop.ptr = top->ptr;
		oldTop.stamp = top->stamp;

		node->next = oldTop.ptr;

		if (InterlockedCompareExchange128(
			(volatile LONG64*)top,
			oldTop.stamp + 1,
			(LONG64)node,
			(LONG64*)&oldTop
		))
		{
			InterlockedIncrement(&size);
			break;
		}
	}
}

bool mymp::LockFreeStack::Pop(BlockHeader*& node)
{
	while (true)
	{
		StampNode oldTop;
		oldTop.ptr = top->ptr;
		oldTop.stamp = top->stamp;
		if (oldTop.ptr == nullptr)
		{
			return false;
		}

		if (InterlockedCompareExchange128(
			(volatile LONG64*)top,
			oldTop.stamp + 1,
			(LONG64)oldTop.ptr->next,
			(LONG64*)&oldTop
		))
		{
			InterlockedDecrement(&size);
			node = oldTop.ptr;
			// TryDelete(oldTop.ptr);

			return true;
		}
	}
}

/*-------------------
	  MemoryPool
-------------------*/

void mymp::MemoryPool::Push(BlockHeader* node)
{
	if (node->size != alloc_size_ || node->code != BlockHeader::CURRENT_ALLOC)
	{
		std::cout << "[MemoryPool] wrong memory release" << std::endl;
		std::cout << "size : " << node->size << "(" << alloc_size_ << "), code : " << node->code << std::endl;
		// CRASH
	}
	else
	{
		// 동일한 메모리에 대해 메모리 해제하는 것을 방지한다.
		if (InterlockedCompareExchange16(&node->code, BlockHeader::CURRENT_RELEASE, BlockHeader::CURRENT_ALLOC) != BlockHeader::CURRENT_ALLOC)
		{
			std::cout << "[MemoryPool] already released memory" << std::endl;
			// CRASH
		}
		else
		{
			container_.Push(node);
			++release_count_;
			--alloc_count_;
		}
	}
}

void mymp::MemoryPool::Pop(BlockHeader*& node)
{
	// 메모리 풀이 비어있는 경우 새로 할당한다.
	if (!container_.Pop(node))
	{
		node = (BlockHeader*)_aligned_malloc(alloc_size_ + sizeof(mymp::BlockHeader), 16);
		if (node == nullptr)
		{
			std::cout << "[MemoryPool] memory alloc failed" << std::endl;
			// CRASH
		}
	}
	else
	{
		if (node->size != alloc_size_ || node->code != BlockHeader::CURRENT_RELEASE)
		{
			std::cout << "[MemoryPool] memory pool corrupted (" << node->code << ")" << std::endl;
			std::cout << "size : " << node->size << ", code : " << node->code << std::endl;
			// CRASH
		}

		--release_count_;
	}

	++alloc_count_;
}