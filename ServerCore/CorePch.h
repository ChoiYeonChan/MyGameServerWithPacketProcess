#pragma once

#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "ws2_32")

#include <WinSock2.h>
#include <Windows.h>

#include <MSWSock.h>
#include <ws2tcpip.h>

using namespace std;

#include <iostream>
#include <functional>
#include <memory>
#include <new>

// STL
#include <utility>

#include <vector>
#include <list>
#include <queue>
#include <map>

// ServerCore
#include "MyMacro.h"
#include "MyTypeDef.h"

#include "SocketUtils.h"

#include "MemoryStream.h"
//#include "RingBuffer.h"

#include "RecvBuffer.h"
#include "SendBuffer.h"


//#include "LockFreeQueue.h"
//#include "LockFreeStack.h"

#include "Allocator.h"
#include "MemoryPool.h"
#include "ObjectPool.h"

// #include "MemoryMacro.h"
//#include "PacketSession.h"