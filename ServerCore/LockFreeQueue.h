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
		// ���ʳ�� ����
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

			if (last.ptr == tail.ptr)	// last.ptr != tail.ptr�̸� �ٸ� �����忡 ���� Enq�� ����Ǿ���.
			{
				if (next == nullptr)	// next != nullptr�̸� �ٸ� �����忡 ���� Enq�� �������̴�.
				{
					// last.ptr->next = node;
					// CAS�� �������� ������ �ٸ� �����忡 ���� next�� ����δ�.
					// �ٸ� �����忡 ���� last.ptr->next�� ��尡 ���� ���� ����� nullptr�� �ƴϹǷ� �����Ѵ�.
					if (InterlockedCompareExchangePointer((volatile PVOID*)&last.ptr->next, node, next) == nullptr)
					{
						// tail = node;
						// CAS�� �������� ������ �ٸ� �����忡 ���� ������ tail�� ��ġ�� ���ƿ´�.
						// ���������� �ٸ� �����忡 ���� tail�� �Ű��� ���̹Ƿ� true�� �����Ѵ�.
						InterlockedCompareExchange128(
							(LONG64*)&tail,
							last.stamp + 1, (LONG64)node,
							(LONG64*)&last);

						InterlockedIncrement64((LONG64*)&m_size);
						return true;
					}
				}
				// next != nullptr �̶�� ���� �ٸ� �����尡 enq�� �õ��ϴ� ��
				// tail.ptr->next = node; ���Ը� �����ϰ� tail�� �ű��� ���� ���¸� �ǹ��ϹǷ�
				// tail�� �ű�� ���� �����ش�.
				else
				{
					InterlockedCompareExchange128(
						(LONG64*)&tail,
						last.stamp + 1, (LONG64)next,	// ���� ���� ���� node�� �ƴ϶� next�ӿ� ��������.
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
					// first.ptr == head.ptr && next == nullptr�̸� ť�� ����ִ� ����̴�.
					if (next == nullptr)
					{
						return false;
					}
					// first.ptr == last.ptr && next != nullptr�̸� �ٸ� �����尡 enq�� �õ��ϴ� ��
					// tail.ptr->next = next; ���Ը� �����ϰ� tail�� �ű��� ���� ���¸� �ǹ��ϹǷ�
					// tail�� �ű�� ���� �����ش�.
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
					// �� ��츦 ��� ó���ߴ��� �ش� ��ġ���� ����ǰ�
					// �ٸ� �����尡 ť�� ���� �׼��� ������ �߻��Ѵ�.
					if (next != nullptr)	// Ȯ�� ���
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