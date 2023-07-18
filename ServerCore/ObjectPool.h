#pragma once

#include "MemoryPool.h"

template <typename Type>
class ObjectPool
{
private:
	static int alloc_size_;
	static mymp::MemoryPool memory_pool_;

public:
	template <typename... Args>
	static Type* Pop(Args&& ... args)
	{
#ifdef __STOMP__
		mymp::BlockHeader* ptr = reinterpret_cast<mymp::BlockHeader*>(StompAllocator::Allocate(alloc_size_ + sizeof(mymp::BlockHeader)));
#else
		mymp::BlockHeader* ptr;
		memory_pool_.Pop(ptr);

#endif
		Type* memory = static_cast<Type*>(mymp::BlockHeader::AttachHeader(ptr, alloc_size_));
		new(memory) Type(std::forward<Args>(args)...);
		return memory;
	}

	static void Push(Type* object)
	{
		object->~Type();
#ifdef __STOMP__
		StompAllocator::Release(mymp::BlockHeader::DetachHeader(object));
#else
		memory_pool_.Push(mymp::BlockHeader::DetachHeader(object));
#endif
	}

	template<typename... Args>
	static std::shared_ptr<Type> MakeShared(Args&&... args)
	{
		std::shared_ptr<Type> ptr{ Pop(std::forward<Args>(args)...), Push };
		return ptr;
	}
};

template <typename Type>
int ObjectPool<Type>::alloc_size_ = sizeof(Type);

template <typename Type>
mymp::MemoryPool ObjectPool<Type>::memory_pool_(alloc_size_);