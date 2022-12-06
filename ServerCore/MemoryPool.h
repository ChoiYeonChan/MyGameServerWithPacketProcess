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
* �޸�Ǯ �ʱ����
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

		// �Ҵ� ����� �ִ� ������� ũ�ٸ� �޸�Ǯ���� �������� �ʰ� �޸� �Ҵ��Ѵ�.
		if (alloc_size > MAX_ALLOC_SIZE)
		{
			alloc_node = (BlockHeader*)AllocateMemory(alloc_size);
		}
		else
		{
			// ó������ ������־��ų� �ٸ� �����忡 ���� ����� ���
			// �޸� Ǯ ������ ����ִٸ� ���ο� �޸𸮸� �Ҵ��Ѵ�.
			if (pool_table_[alloc_size].Pop(alloc_node) == false)
			{
				std::cout << "[MemoryPool] memory block pop failed" << std::endl;
				alloc_node = (BlockHeader*)AllocateMemory(alloc_size);
			}
			else
			{
				// pop�� ���������� ����� �ڵ尡 RELEASE_CODE�� �ƴϰų� size�� �ٸ��ٸ� �޸�Ǯ�� ������ ���̴�.
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

		// ����� �ڵ尡 ALLOC_CODE�� �ƴ϶�� �߸��� �޸𸮸� �ݳ��� ���̴�.
		if (release_node->code != ALLOC_CODE)
		{
			std::cout << "[MemoryPool] not my memory, node code : " << release_node->code << std::endl;
			// CRASH("NOT MY MEMORY");
			return false;
		}
		else
		{
			// ����� �ڵ带 RELEASE_CODE�� �����Ѵ�. ������ �޸𸮿� ���� ���� �õ��ϴ� ��츦 �����Ѵ�.
			if (InterlockedCompareExchange16(&release_node->code, RELEASE_CODE, ALLOC_CODE) != ALLOC_CODE)
			{
				std::cout << "[MemoryPool] already released node" << std::endl;
				return false;
			}

			int alloc_size = release_node->size;
			// �ִ� ������� Ŭ ���� �޸� Ǯ�� �ݳ����� �ʰ� �޸� �����Ѵ�.
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

