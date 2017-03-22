#include "TcpSocket.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

TcpSocket::TcpSocket(std::string ipAddress, uint16_t port) : Socket()
{
	socketDescriptor_ = socket(AF_INET, SOCK_STREAM, 0);
	if (socketDescriptor_ < 0)
	{
		std::cout << "Error: " << strerror(errno);
		valid_ = false;
	}
	else
	{
		address_.sin_family = AF_INET;
		inet_aton(ipAddress.c_str(), &(address_.sin_addr));
		address_.sin_port = htons(port);
		if (connect(socketDescriptor_, (struct sockaddr*) &address_, sizeof(address_)) < 0)
		{
			valid_ = false;
		}
	}
}

TcpSocket::~TcpSocket()
{
	if (valid_)
	{
		shutdown(socketDescriptor_, 2);
		::close(socketDescriptor_);
	}
}

Socket::SocketType TcpSocket::getSocketType()
{
	return Socket::SocketType::TCP;
}

ssize_t TcpSocket::send(const std::string& txt)
{
	return send(txt.c_str());
}

ssize_t TcpSocket::send(const char* txt)
{
	return send((uint8_t*)txt, strlen(txt));
}

ssize_t TcpSocket::send(const char* data, size_t len)
{
	return send((uint8_t*) data, len);
}

ssize_t TcpSocket::send(const uint8_t* data, size_t len)
{
	if(!valid_)
	{
		return -1;
	}
	return sendto(socketDescriptor_, data, len, 0, (struct sockaddr *)&address_, sizeof(address_));
}

void TcpSocket::recv()
{
	char buff[512];
	if (valid_)
	{
		::recv(socketDescriptor_, (void*)buff, sizeof(buff), 0);
	}
}

void TcpSocket::close()
{
	::close(socketDescriptor_);
	valid_ = false;
}