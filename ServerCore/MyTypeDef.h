#pragma once
#include "pch.h"

/*********************
*	  base types
**********************/
using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;


/*********************
*	  shared_ptr
**********************/
using ServiceRef = std::shared_ptr<class Service>;
using ServerServiceRef = std::shared_ptr<class ServerService>;

using SessionManagerRef = std::shared_ptr<class SessionManager>;
using IocpManagerRef = std::shared_ptr<class IocpManager>;

using IocpObjectRef = std::shared_ptr<class IocpObject>;
using ListenerRef = std::shared_ptr<class Listener>;

using SessionRef = std::shared_ptr<class Session>;
using PacketSessionRef = std::shared_ptr<class PacketSession>;

using SendBufferRef = std::shared_ptr<class SendBuffer>;
using BufferChunkRef = std::shared_ptr<class BufferChunk>;
