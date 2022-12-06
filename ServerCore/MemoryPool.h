#pragma once

#include <Windows.h>
#include <atomic>
#include <memory>
#include <iostream>

namespace mymp
{
	struct BlockHeader
	{
		static const int CURRENT_ALLOC = 0x100;
		static const int CURRENT_RELEASE = 0x200;

		BlockHeader(short code, short size) : code(code), size(size), next(nullptr) {};

		short code;
		short size;

		BlockHeader* next;

		static void* AttachHeader(BlockHeader* header, short size)
		{
			new(header) BlockHeader(CURRENT_ALLOC, size);
			return (void*)(++header);
		}

		static BlockHeader* DetachHeader(void* ptr)
		{
			return (BlockHeader*)(ptr) - 1;
		}
	};

	class LockFreeStack
	{
	private:
		struct __declspec(align(16)) StampNode
		{
			BlockHeader* ptr;
			LONGLONG stamp;

			StampNode() : ptr(nullptr), stamp(0) { }
		};

	private:
		StampNode* top;
		ULONGLONG size;

	public:
		LockFreeStack();
		~LockFreeStack();

		void Clear();

		void Push(BlockHeader* node);
		bool Pop(BlockHeader*& node);

		ULONGLONG Size() { return size; }
		bool Empty() { return (size == 0 && top->ptr == nullptr); }
	};

	class MemoryPool
	{
	private:
		const unsigned int alloc_size_;

		LockFreeStack container_;	// memory block container

		std::atomic<int> alloc_count_;
		std::atomic<int> release_count_;

	public:
		MemoryPool(unsigned int alloc_size) : alloc_size_(alloc_size), alloc_count_(0), release_count_(0) { }
		~MemoryPool() { }

		void Push(BlockHeader* node);
		void Pop(BlockHeader*& node);

		int AllocCount() const { return alloc_count_; }
		int ReleaseCount() const { return release_count_; }

		int AllocSize() const { return alloc_size_; }
	};
}


/*
* 메모리풀 초기버전
*
class MemoryPool
{
private:
	static const int ALLOC_CODE = 0x100;
	static const int RELEASE_CODE = 0x200;
	static const int MAX_ALLOC_SIZE = 50;

	LockFreeStack pool_table_[MAX_ALLOC_SIZE];
	LONG64 count_alloc_;

public:
	MemoryPool() : count_alloc_(0)
	{
	}

	~MemoryPool()
	{
		// Clear();
	}

	void* Allocate(unsigned int size)
	{
		BlockHeader* alloc_node = nullptr;
		int alloc_size = size + sizeof(BlockHeader);

		// 할당 사이즈가 최대 사이즈보다 크다면 메모리풀에서 추출하지 않고 메모리 할당한다.
		if (alloc_size > MAX_ALLOC_SIZE)
		{
			alloc_node = (BlockHeader*)AllocateMemory(alloc_size);
		}
		else
		{
			// 처음부터 비워져있었거나 다른 스레드에 의해 비워진 경우
			// 메모리 풀 스택이 비어있다면 새로운 메모리를 할당한다.
			if (pool_table_[alloc_size].Pop(alloc_node) == false)
			{
				std::cout << "[MemoryPool] memory block pop failed" << std::endl;
				alloc_node = (BlockHeader*)AllocateMemory(alloc_size);
			}
			else
			{
				// pop에 성공했지만 노드의 코드가 RELEASE_CODE가 아니거나 size가 다르다면 메모리풀이 오염된 것이다.
				if (alloc_node->code != RELEASE_CODE || alloc_node->size != alloc_size)
				{
					std::cout << "[MemoryPool] wrong node exist, code : " << alloc_node->code << ", size : " << alloc_node->size << std::endl;
					// CRASH("MEMORY POOL CORRUPTED");
				}
			}
		}

		if (alloc_node == nullptr)
		{
			std::cout << "[MemoryPool] memory allocate failed" << std::endl;
			return nullptr;
		}

		alloc_node->code = ALLOC_CODE;
		alloc_node->size = alloc_size;
		InterlockedIncrement64(&count_alloc_);

		return (void*)(++alloc_node);
	}

	bool Release(void* ptr)
	{
		BlockHeader* release_node = (BlockHeader*)(ptr) - 1;

		// 노드의 코드가 ALLOC_CODE가 아니라면 잘못된 메모리를 반납한 것이다.
		if (release_node->code != ALLOC_CODE)
		{
			std::cout << "[MemoryPool] not my memory, node code : " << release_node->code << std::endl;
			// CRASH("NOT MY MEMORY");
			return false;
		}
		else
		{
			// 노드의 코드를 RELEASE_CODE로 변경한다. 동일한 메모리에 대해 해제 시도하는 경우를 방지한다.
			if (InterlockedCompareExchange16(&release_node->code, RELEASE_CODE, ALLOC_CODE) != ALLOC_CODE)
			{
				std::cout << "[MemoryPool] already released node" << std::endl;
				return false;
			}

			int alloc_size = release_node->size;
			// 최대 사이즈보다 클 경우는 메모리 풀로 반납하지 않고 메모리 해제한다.
			if (alloc_size > MAX_ALLOC_SIZE)
			{
				_aligned_free(release_node);
			}
			else
			{
				pool_table_[alloc_size].Push(release_node);
			}

			InterlockedDecrement64(&count_alloc_);
			return true;
		}
	}

	void DisplayPoolTableCount()
	{
		for (int i = 1; i < MAX_ALLOC_SIZE; i++)
		{
			std::cout << pool_table_[i].GetSize() << " ";
			if (i % 10 == 0)
				std::cout << std::endl;
		}
		std::cout << std::endl;
	}

private:
	void* AllocateMemory(unsigned int size)
	{
		void* ptr = _aligned_malloc(size, 16);
		return ptr;
	}
};
*/

