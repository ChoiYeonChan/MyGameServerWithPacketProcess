#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

void* BaseAllocator::Allocate(unsigned int size)
{
    return malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
    free(ptr);
}

void* StompAllocator::Allocate(unsigned int size)
{
    int page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    int data_offset = page_count * PAGE_SIZE - size;
    void* base_address = VirtualAlloc(NULL, page_count * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return static_cast<void*>(static_cast<char*>(base_address) + data_offset);
}

void StompAllocator::Release(void* ptr)
{
    __int64 address = reinterpret_cast<__int64>(ptr);
    __int64 base_address = address - (address % PAGE_SIZE);
    VirtualFree(reinterpret_cast<void*>(base_address), 0, MEM_RELEASE);
}

void* PoolAllocator::Allocate(unsigned int size)
{
    return g_memory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
    g_memory->Release(ptr);
}
