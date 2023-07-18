#pragma once

/*********************
*	     Crash
**********************/

#define CRASH(cause)								\
{													\
	unsigned int* crash = nullptr;					\
	__analysis_assume(crash != nullptr);			\
	*crash = 0x12345678;							\
}

#define ASSERT_CRASH(expr)							\
{													\
	if (!(expr))									\
	{												\
		CRASH("ASSERT_CRASH")						\
		__analysis_assume(expr);					\
	}												\
}


/*********************
*	     Lock
**********************/

#define USE_MANY_LOCKS(count)	Lock locks_[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define READ_LOCK_IDX(idx)		ReadLockGuard read_lock_guard_##idx(locks_[idx]);
#define READ_LOCK				READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)		WriteLockGuard write_lock_guard_##idx(locks_[idx]);
#define WRITE_LOCK				WRITE_LOCK_IDX(0)


/*********************
*	     Size
**********************/

#define size32(val) static_cast<int>(sizeof(val))
#define len32(arr) static_cast<int>(sizeof(arr) / sizeof(arr[0]))