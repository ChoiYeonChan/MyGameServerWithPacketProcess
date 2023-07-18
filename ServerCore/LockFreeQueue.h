#pragma once
#include "pch.h"

template <typename DATA>
class LockFreeQueue
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
	StampNode head;
	StampNode tail;
	ULONGLONG size;

	Node* free_list;
	ULONGLONG pop_count;

public:
	LockFreeQueue() : size(0)
	{
		head.ptr = tail.ptr = (Node*)_aligned_malloc(sizeof(Node), 16);
		if (head.ptr == nullptr || tail.ptr == nullptr)
		{
			std::cout << "메모리 할당에 실패했습니다. (head / tail)" << std::endl;
			return;
		}

		head.ptr->next = tail.ptr->next = nullptr;
		head.stamp = tail.stamp = 0;
	}

	~LockFreeQueue()
	{
		Clear();
		_aligned_free(head.ptr);
		// tail.ptr을 free하면 중복 해제가 되어버림
	}

	bool Enqueue(const DATA source)
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
			StampNode last;
			Node* next;
			last = tail;
			next = last.ptr->next;

			if (last.ptr == tail.ptr)
			{
				if (next == nullptr)
				{
					// last.ptr->next = node;
					// 반환값이 nullptr이 아니라는 것은 다른 스레드에 의해 last.ptr->next가 바꼈다는 것이다.
					if (InterlockedCompareExchangePointer((PVOID*)&last.ptr->next, node, next) == nullptr)
					{
						// tail = node;
						// 해당 위치에서 스레드가 sleep되면 단위 시간 당 작업은 0이 되므로 다른 스레드가 도와준다.
						InterlockedCompareExchange128((LONG64*)&tail, last.stamp + 1, (LONG64)node, (LONG64*)&last);

						// tail = node; 가 어느 위치에서 성공했든 카운트는 여기서만 오른다.
						InterlockedIncrement64((LONG64*)&size);
						return true;
					}
				}
				else
				{
					// 대신해서 tail = next를 수행 (이 때, 내가 만든 node가 아님에 유의한다.)
					InterlockedCompareExchange128((LONG64*)&tail, last.stamp + 1, (LONG64)next, (LONG64*)&last);
				}
			}
		}
	}

	bool Dequeue(DATA* destination)
	{
		InterlockedIncrement(&pop_count);

		while (true)
		{
			StampNode first = head;
			StampNode last = tail;
			Node* next = first.ptr->next;

			if (first.ptr == head.ptr)
			{
				if (first.ptr == last.ptr)
				{
					if (next == nullptr)
					{
						return false;
					}
					else
					{
						InterlockedCompareExchange128((LONG64*)&tail, last.stamp + 1, (LONG64)next, (LONG64*)&last);
					}
				}
				else
				{
					if (InterlockedCompareExchange128((LONG64*)&head, first.stamp + 1, (LONG64)next, (LONG64*)&first))
					{
						*destination = next->data;

						TryDelete(first.ptr);
						first.ptr = nullptr;

						InterlockedDecrement64((LONG64*)&size);
						return true;
					}
				}
			}
		}
	}

	void Clear()
	{
		unsigned long long cnt = 0;

		Node* ptr = nullptr;
		while (head.ptr->next != nullptr)
		{
			ptr = head.ptr->next;
			head.ptr->next = head.ptr->next->next;
			_aligned_free(ptr);

			InterlockedDecrement(&size);
			InterlockedIncrement(&cnt);
		}

		head.stamp = 0;
		tail = head;

		std::cout << "LockFreeQueue Clear Count : " << cnt << " Size : " << size << std::endl;
	}

	ULONGLONG GetSize() { return size; }
	bool Empty() { return (head.ptr == tail.ptr && head.ptr->next == nullptr); }

	void Display(int count)
	{
		printf("\n==== Display ==== \n");

		Node* ptr = head.ptr->next;
		while (count > 0 && ptr != nullptr)
		{
			std::cout << ptr->data << " ";
			ptr = ptr->next;
			--count;
		}
		printf("\n");
	}

private:
	void TryDelete(Node* node)
	{
		if (pop_count == 1)
		{
			// 내가 head를 옮기고 난 이후 TryDelete를 하기 때문에 해당 시점에 pop_count가 1이라는 것은 
			// 다른 스레드가 같은 head를 가지고 있지 않다는 것이다.
			_aligned_free(node);

			// 이전에 모아둔 pending list에 대해 메모리 해제를 시도한다.			
			Node* old_free_list = (Node*)InterlockedExchangePointer((PVOID*)(&free_list), nullptr);
			if (InterlockedDecrement(&pop_count) == 0)
			{
				// 해당 시점에 pop_count가 0이라는 것은 다른 스레드가 메모리 해제가 필요한 노드를 모두 리스트에 삽입하였거나
				// 아무런 시도도 일어나지 않아 위에서 분리한 old_free_list를 메모리 해제할 수 있다는 것이다.
				ReleaseFreeList(old_free_list);
			}
			else if (old_free_list != nullptr)
			{
				AddListToFreeList(old_free_list);
			}
		}
		else
		{
			AddNodeFreeList(node);
			InterlockedDecrement(&pop_count);
		}
	}

	void AddNodeFreeList(Node* first, Node* last)
	{
		while (true)
		{
			Node* old_free_list = free_list;
			last->next = old_free_list;
			if (InterlockedCompareExchangePointer((PVOID*)&free_list, first, old_free_list) == old_free_list)
			{
				return;
			}
		}
	}

	void AddNodeFreeList(Node* node)
	{
		AddNodeFreeList(node, node);
	}

	void AddListToFreeList(Node* first)
	{
		Node* last = first;
		while (last->next != nullptr)
		{
			last = last->next;
		}
		AddNodeFreeList(first, last);
	}

	void ReleaseFreeList(Node* node)
	{
		while (node != nullptr)
		{
			Node* next = node->next;
			_aligned_free(node);
			node = next;
		}
	}
};

