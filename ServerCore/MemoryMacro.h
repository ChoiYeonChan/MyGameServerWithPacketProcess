#pragma once

#include "pch.h"
#include "Allocator.h"

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
	PoolAllocator::Release(object);
}

template <typename Type, typename... Args>
std::shared_ptr<Type> MakeShared(Args&&... args)
{
	return std::shared_ptr<Type>{ xnew<Type>(std::forward<Args>(args)...), xdelete<Type> };
}