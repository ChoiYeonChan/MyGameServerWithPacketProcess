#pragma once

#include <new>

namespace mymp
{
	struct BlockHeader
	{
	public:
		static const int CURRENT_ALLOC = 0x100;
		static const int CURRENT_RELEASE = 0x200;

		short code;
		short size;

		BlockHeader* next;

		BlockHeader(short code, short size) : code(code), size(size), next(nullptr) { };

		static void* AttachHeader(BlockHeader* header, unsigned short size)
		{
			new(header) BlockHeader(CURRENT_ALLOC, size);
			return (void*)(++header);
		}

		static BlockHeader* DetachHeader(void* ptr)
		{
			return (BlockHeader*)(ptr)-1;
		}
	};

	class LockFreeStack
	{
	private:
		struct __declspec(align(16)) StampNode
		{
			BlockHeader* ptr;
			LONG64 stamp;

			StampNode() : ptr(nullptr), stamp(0) { }
		};

	private:
		StampNode* top;
		ULONGLONG size;

	public:
		LockFreeStack();
		~LockFreeStack();

		void Clear();

		void Push(BlockHeader* header);
		bool Pop(BlockHeader*& header);

		ULONGLONG Size() { return size; }
		bool Empty() { return (size == 0 && top->ptr == nullptr); }
	};

	class MemoryPool
	{
	private:
		const short alloc_size_;

		mymp::LockFreeStack container_;		// memory header block container

		std::atomic<int> alloc_count_;
		std::atomic<int> release_count_;

	public:
		MemoryPool(short alloc_size) : alloc_size_(alloc_size), alloc_count_(0), release_count_(0) { }
		~MemoryPool() { }

		void Push(BlockHeader* header);
		void Pop(BlockHeader*& header);

		int AllocCount() const { return alloc_count_; }
		int ReleaseCount() const { return release_count_; }

		int AllocSize() const { return alloc_size_; }

	};
}

