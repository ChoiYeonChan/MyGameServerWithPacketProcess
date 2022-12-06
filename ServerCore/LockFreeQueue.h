#pragma once
#include <Windows.h>
#include <cstdio>
#include <iostream>
#include "MemoryPool.h"

using namespace std;

template <typename DATA>
class LockFreeQueue
{
private:
	struct __declspec(align(16)) Node
	{
		DATA value;
		Node* next;

		Node() : next(nullptr) { }
		Node(DATA _value) : value(_value), next(nullptr) { }
	};

	struct __declspec(align(16)) StampNode
	{
		Node* ptr;
		LONGLONG stamp;

		StampNode() : ptr(nullptr), stamp(0) { }
		StampNode(Node* _ptr, __int64 _stamp) : ptr(_ptr), stamp(_stamp) { }
	};

private:
	StampNode head;
	StampNode tail;

	unsigned int m_size;

	MemoryPool m_pool;

public:
	LockFreeQueue() : m_size(0)
	{
		// 보초노드 생성
		head.ptr = tail.ptr = xnew<Node>("LockFreeQueue()");
	}

	~LockFreeQueue()
	{
		Clear();
		xdelete(head.ptr);
	}

	void Clear()
	{
		Node* ptr = nullptr;
		volatile unsigned long long cnt = 0;
		while (head.ptr->next != nullptr)
		{
			InterlockedIncrement(&cnt);
			ptr = head.ptr->next;
			head.ptr->next = head.ptr->next->next;
			xdelete(ptr);
		}

		head.stamp = 0;
		tail = head;
		m_size = 0;

		cout << "lockfree queue clear count : " << cnt << endl;
	}

	bool Enqueue(DATA value)
	{
		Node* node = xnew<Node>("Enqueue()", value);
		if (node == nullptr)
		{
			CRASH("MEMORY ALLOCATE FAILED");
			return false;
		}

		while (true)
		{
			StampNode last;
			Node* next;
			last = tail;
			next = last.ptr->next;

			if (last.ptr == tail.ptr)	// last.ptr != tail.ptr이면 다른 스레드에 의해 Enq가 수행되었다.
			{
				if (next == nullptr)	// next != nullptr이면 다른 스레드에 의해 Enq가 실행중이다.
				{
					// last.ptr->next = node;
					// CAS로 수행하지 않으면 다른 스레드에 의해 next가 덮어쓰인다.
					// 다른 스레드에 의해 last.ptr->next에 노드가 들어가면 리턴 결과가 nullptr이 아니므로 실패한다.
					if (InterlockedCompareExchangePointer((volatile PVOID*)&last.ptr->next, node, next) == nullptr)
					{
						// tail = node;
						// CAS로 수행하지 않으면 다른 스레드에 의해 지정된 tail의 위치가 돌아온다.
						// 실패했으면 다른 스레드에 의해 tail이 옮겨진 것이므로 true를 리턴한다.
						InterlockedCompareExchange128(
							(LONG64*)&tail,
							last.stamp + 1, (LONG64)node,
							(LONG64*)&last);

						InterlockedIncrement64((LONG64*)&m_size);
						return true;
					}
				}
				// next != nullptr 이라는 것은 다른 스레드가 enq를 시도하던 중
				// tail.ptr->next = node; 삽입만 수행하고 tail로 옮기지 못한 상태를 의미하므로
				// tail을 옮기는 것을 도와준다.
				else
				{
					InterlockedCompareExchange128(
						(LONG64*)&tail,
						last.stamp + 1, (LONG64)next,	// 내가 새로 만든 node가 아니라 next임에 유의하자.
						(LONG64*)&last);
				}
			}
		}

		return true;
	}

	bool Dequeue(DATA* data)
	{
		StampNode first;
		StampNode last;
		Node* next;

		DATA ret;

		while (true)
		{
			first = head;
			last = tail;
			next = first.ptr->next;

			if (first.ptr == head.ptr)
			{
				if (first.ptr == last.ptr)
				{
					// first.ptr == head.ptr && next == nullptr이면 큐가 비어있는 경우이다.
					if (next == nullptr)
					{
						return false;
					}
					// first.ptr == last.ptr && next != nullptr이면 다른 스레드가 enq를 시도하던 중
					// tail.ptr->next = next; 삽입만 수행하고 tail로 옮기지 못한 상태를 의미하므로
					// tail을 옮기는 것을 도와준다.
					else
					{
						InterlockedCompareExchange128(
							(LONG64*)&tail,
							last.stamp + 1, (LONG64)next,
							(LONG64*)&last
						);
					}
				}
				else
				{
					// 빈 경우를 모두 처리했더라도 해당 위치에서 블락되고
					// 다른 스레드가 큐를 비우면 액세스 위반이 발생한다.
					if (next != nullptr)	// 확인 요망
					{
						ret = next->value;
						if (InterlockedCompareExchange128(
							(LONG64*)&head,
							first.stamp + 1, (LONG64)next,
							(LONG64*)&first))
						{
							xdelete(first.ptr);
							first.ptr = nullptr;
							*data = ret;

							InterlockedDecrement64((LONG64*)&m_size);
							return true;
						}
					}
				}
			}
		}
	}

	int GetSize() const { return m_size; }

	bool Empty() const {
		if (m_size == 0) return true;
		else return false;
	}

	void Display(int count)
	{
		printf("\n==== Display ==== \n");

		Node* p = head.ptr->next;
		while (count > 0 && p != nullptr)
		{
			printf("%d, ", p->value);
			p = p->next;
			--count;
		}
		printf("\n");
	}
};