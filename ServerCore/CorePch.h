#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "ws2_32")
#include <WinSock2.h>
#include <Windows.h>

#include <MSWSock.h>
#include <ws2tcpip.h>

using namespace std;

// C++
#include <iostream>
#include <functional>
#include <memory>
#include <new>

#include <thread>
#include <mutex>
#include <atomic>

// STL
#include <utility>

#include <array>
#include <vector>
#include <list>
#include <queue>
#include <map>

// ServerCore

// Main
#include "MyMacro.h"
#include "MyTypeDef.h"

// Memory
#include "Memory.h"
#include "MemoryPool.h"
#include "ObjectPool.h"
#include "Allocator.h"
#include "MemoryMacro.h"

// Network
#include "SocketUtils.h"
#include "Session.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"

#include "Service.h"

// Utils
#include "MemoryStream.h"
#include "LockFreeQueue.h"
#include "LockFreeStack.h"
// #include "RingBuffer.h"