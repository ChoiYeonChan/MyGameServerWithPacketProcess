#pragma once

class BaseAllocator
{
public:
	static void* Allocate(unsigned int size);
	static void Release(void* ptr);
};

class StompAllocator
{
private:
	enum { PAGE_SIZE = 0x1000 };

public:
	static void* Allocate(unsigned int size);
	static void Release(void* ptr);
};

class PoolAllocator
{
public:
	static void* Allocate(unsigned int size);
	static void Release(void* ptr);
};

template <typename T>
class StlAllocator
{
public:
	using value_type = T;
	using size_type = size_t;

public:
	StlAllocator() {}

	template <typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	T* allocate(size_t count)
	{
		int size = static_cast<int>(count * sizeof(T));
		return static_cast<T*>(StompAllocator::Allocate(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		StompAllocator::Release(ptr);
	}
};