#pragma once

#include <iostream>
#include <Windows.h>
#include <new>
#include <malloc.h>

using namespace std;

template <typename DATA>
class LockFreeStack
{
private:
	struct __declspec(align(16)) Node
	{
		DATA value;
		Node* next;

		Node() : value(0), next(nullptr) { }
		Node(DATA _value) : value(_value), next(nullptr) { }
	};

	struct __declspec(align(16)) StampNode
	{
		Node* ptr;
		LONGLONG stamp;

		StampNode() : ptr(nullptr), stamp(0) { }
	};

private:
	volatile StampNode* top;
	Node* free_list;
	ULONGLONG size;
	ULONGLONG pop_count;

public:
	LockFreeStack() : free_list(nullptr), size(0), pop_count(0)
	{
		top = (StampNode*)_aligned_malloc(sizeof(StampNode), 16);
		if (top == nullptr)
		{
			std::cout << "LockFreeStack() : top is nullptr" << std::endl;
		}
		top->ptr = nullptr;
		top->stamp = 0;
	}

	~LockFreeStack()
	{
		Clear();
	}

	void Clear()
	{
		volatile unsigned long long cnt = 0;

		Node* ptr = nullptr;
		while (top->ptr != nullptr)
		{
			ptr = top->ptr;
			top->ptr = top->ptr->next;
			_aligned_free(ptr);
			InterlockedDecrement(&size);

			InterlockedIncrement(&cnt);
		}
		InterlockedExchange64(&top->stamp, 0);

		std::cout << "LockFreeStack clear count : " << cnt << " size : " << size << std::endl;
	}

	bool Push(const DATA& _value)
	{
		Node* node = (Node*)_aligned_malloc(sizeof(Node), 16);
		if (node == nullptr)
		{
			std::cout << "memory nullptr\n";
			return false;
		}
		new(node) Node(_value);

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
				return true;
			}
		}
	}

	bool Pop(DATA* _value)
	{
		InterlockedIncrement(&pop_count);

		while (true)
		{
			StampNode oldTop;
			oldTop.ptr = top->ptr;
			oldTop.stamp = top->stamp;
			if (oldTop.ptr == nullptr)
			{
				InterlockedDecrement(&pop_count);
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
				*_value = oldTop.ptr->value;
				TryDelete(oldTop.ptr);

				return true;
			}
		}
	}

	ULONGLONG GetSize()
	{
		return size;
	}

	bool Empty()
	{
		return (size == 0 && top->ptr == nullptr);
	}

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

private:

	void TryDelete(Node* oldTop)
	{
		/* TryDelete를 수행하는 시점에 pop_count가 1이라는 것은
		 * 내가 delete할 top를 oldTop으로 가지고 있는 스레드는 없다는 의미이다.
		 * 그러므로 top은 분명하게 delete 할 수 있다.
		 *
		 */

		if (pop_count == 1)
		{
			_aligned_free(oldTop);

			Node* old_free_list = (Node*)InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&free_list), NULL);

			if (InterlockedDecrement(&pop_count) == 0)
			{
				ReleaseFreeList(old_free_list);
			}
			else if (old_free_list != nullptr)
			{
				AddListToFreeList(old_free_list);
			}
		}
		/* pop_count가 1이 아니라는 것은 내가 delete할 top을 다른 스레드가
		 * oldTop으로 가지고 있다는 것이므로 삭제 예약만 한다.
		 */
		else
		{
			AddNodeFreeList(oldTop);
			InterlockedDecrement(&pop_count);
		}
	}

	void AddNodeFreeList(Node* first, Node* last)
	{
		while (true)
		{
			Node* old_free_list = free_list;
			last->next = old_free_list;
			if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&free_list), first, old_free_list)
				== old_free_list)
			{
				return;
			}
		}
	}

	void AddNodeFreeList(Node* node)
	{
		AddNodeFreeList(node, node);
	}

	void AddListToFreeList(Node* node)
	{
		Node* last = node;
		while (last->next != nullptr)
		{
			last = last->next;
		}
		AddNodeFreeList(node, last);
	}

	void ReleaseFreeList(Node* ptr)
	{
		while (ptr != nullptr)
		{
			Node* next = ptr->next;
			_aligned_free(ptr);
			ptr = next;
		}
	}
};

