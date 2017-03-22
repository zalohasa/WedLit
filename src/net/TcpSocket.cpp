#include "TcpSocket.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>


static void charPointerDeleter(const char* pointer)
{
	free((void*) pointer);
}

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
		shutdown(socketDescriptor_, SHUT_RDWR);
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
	return ::send(socketDescriptor_, (void*)data, len, 0);
}

TcpSocket::TcpData TcpSocket::recv(int timeout)
{
	TcpSocket::TcpData data;
	if (!valid_)
	{
		return data;
	}
	
	struct pollfd fds[1];
	fds[0].fd = socketDescriptor_;
	fds[0].events = POLLIN;

	if (poll(fds, 1, timeout) > 0)
	{
		data.timestamp = std::chrono::steady_clock::now();
		char buff[512];
		ssize_t res = ::recv(socketDescriptor_, (void*)buff, sizeof(buff), 0);
		if (res > 0)
		{
			char* raw = (char*) malloc(res);
			memcpy(raw, buff, res);
			std::function<void(const char*)> deleter = charPointerDeleter;
			data.data = std::shared_ptr<const char>(raw, deleter);
			data.size = res;
		}
	}
	return data;
}

void TcpSocket::close()
{
	if (valid_)
	{
		::close(socketDescriptor_);
		valid_ = false;
	}
}