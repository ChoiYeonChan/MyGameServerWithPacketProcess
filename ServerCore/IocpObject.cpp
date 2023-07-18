#include "pch.h"
#include "IocpObject.h"

void IocpObject::SocketIOErrorHandler(int error_code)
{
	switch (error_code)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		std::cout << "IOCP Object Closed (errorCode : " << error_code << ")" << std::endl;
		this->Close();
		break;
	default:
		// TODO : Log
		std::cout << "SocketIOError : " << error_code << std::endl;
		break;
	}
}