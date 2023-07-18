#pragma once
#include "pch.h"


template <typename DATA>
class LockFreeStack
{
private:
	struct __declspec(align(16)) Node
	{
		DATA data;
		Node* next;

		Node() : next(nullptr) { }
		Node(DATA value) : data(value), next(nullptr) { }
	};

	struct __declspec(align(16)) StampNode
	{
		Node* ptr;
		LONG64 stamp;

		StampNode() : ptr(nullptr), stamp(0) { }
	};

private:
	StampNode* top;
	ULONGLONG size;

	Node* free_list;
	ULONGLONG pop_count;

public:
	LockFreeStack() : free_list(nullptr), size(0), pop_count(0)
	{
		top = (StampNode*)_aligned_malloc(sizeof(StampNode), 16);
		if (top == nullptr)
		{
			std::cout << "메모리 할당에 실패했습니다. (top)" << std::endl;
			return;
		}

		top->ptr = nullptr;
		top->stamp = 0;
	}

	~LockFreeStack()
	{
		Clear();
		_aligned_free(top);
	}

	bool Push(const DATA source)
	{
		Node* node = (Node*)_aligned_malloc(sizeof(Node), 16);
		if (node == nullptr)
		{
			std::cout << "메모리 할당에 실패했습니다. (node)" << std::endl;
			return false;
		}
		new(node) Node(source);

		while (true)
		{
			StampNode oldTop;
			oldTop.ptr = top->ptr;
			oldTop.stamp = top->stamp;

			node->next = oldTop.ptr;
			if (InterlockedCompareExchange128((LONG64*)top, oldTop.stamp + 1, (LONG64)node, (LONG64*)&oldTop))
			{
				InterlockedIncrement(&size);
				return true;
			}
		}
	}

	bool Pop(DATA* destination)
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

			if (InterlockedCompareExchange128((LONG64*)top, oldTop.stamp + 1, (LONG64)oldTop.ptr->next, (LONG64*)&oldTop))
			{
				InterlockedDecrement(&size);
				*destination = oldTop.ptr->data;

				return true;
			}
		}
	}

	void Clear()
	{
		unsigned long long cnt = 0;

		Node* ptr = nullptr;
		while (top->ptr != nullptr)
		{
			ptr = top->ptr;
			top->ptr = top->ptr->next;
			_aligned_free(ptr);

			InterlockedDecrement(&size);
			InterlockedIncrement(&cnt);
		}

		top->ptr = nullptr;
		top->stamp = 0;

		std::cout << "LockFreeStack Clear Count : " << cnt << " Size : " << size << std::endl;
	}

	ULONGLONG GetSize() { return size; }
	bool Empty() { return (size == 0 && top->ptr == nullptr); }

	void Display(int cnt)
	{
		Node* ptr = top->ptr;
		while (ptr != nullptr && cnt > 0)
		{
			std::cout << ptr->value << " ";
			ptr = ptr->next;
			--cnt;
		}
		std::cout << "\n difference : " << cnt;
	}
};

