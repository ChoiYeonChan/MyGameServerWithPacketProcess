#pragma once

#include "pch.h"
#include "Allocator.h"

/*
#ifdef __STOMP__
#define xalloc(size) StompAllocator::Allocate(size)
#define xrelease(ptr) StompAllocator::Release(ptr)
#else
#define xalloc(size) PoolAllocator::Allocate(size)
#define xrelease(ptr) PoolAllocator::Release(ptr)
#endif
*/

/*
1. placement-new 를 일일이 해줘야되는 점
2. 데이터 타입마다 받는 인자의 타입이랑 갯수가 다른 점
3. 우측값과 좌측값을 동시에 받아야 한다는 점
4. xnew (함수) 안에서 함수를 호출할 때 우측값 레퍼런스가 좌측값으로 전달된다는 점
*/

template <typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* ptr = static_cast<Type*>(PoolAllocator::Allocate(sizeof(Type)));
	new(ptr) Type(std::forward<Args>(args)...);
	return ptr;
}

template <typename Type>
void xdelete(Type* object)
{
	object->~Type();
	PoolAllocator::Release(object);
}

template <typename Type, typename... Args>
std::shared_ptr<Type> MakeShared(Args&&... args)
{
	return std::shared_ptr<Type>{ xnew<Type>(std::forward<Args>(args)...), xdelete<Type> };
}