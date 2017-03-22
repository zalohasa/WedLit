#include "UdpSocket.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <poll.h>
#include <stdlib.h>
#include <functional>

static long getTimestamp()
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	std::cout << "Time: " << time.tv_nsec << std::endl;
	return (time.tv_sec * 1000) + (time.tv_nsec / (1000*1000));
}

void charPointerDeleter(const char* data)
{
	free((void*)data);
}

UdpSocket::UdpSocket(std::string ipAddress, uint16_t port, bool broadcast) : Socket()
{
	socketDescriptor_ = socket(AF_INET, SOCK_DGRAM, 0);
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

		if (broadcast)
		{
			int enable = 1;
			setsockopt(socketDescriptor_, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
		}
	}
}

UdpSocket::~UdpSocket()
{
	if (valid_)
	{
		shutdown(socketDescriptor_, 2);//shutdown transmission and reception
		close(socketDescriptor_);
	}
}

Socket::SocketType UdpSocket::getSocketType()
{
	return Socket::SocketType::UDP;
}

void UdpSocket::send(const std::string& txt)
{
	send(txt.c_str());
}

void UdpSocket::send(const char* txt)
{
	send((uint8_t*)txt, strlen(txt));
}

void UdpSocket::send(const uint8_t* data, size_t len)
{
	if(!valid_)
	{
		return;
	}
	ssize_t writed = sendto(socketDescriptor_, data, len, 0, (struct sockaddr *)&address_, sizeof(address_));
	if (writed < 0)
	{
		std::cout << "Error while sending the data" << std::endl;
	}
}

UdpSocket::UdpResponse UdpSocket::recvfrom_one(int timeout)
{
	UdpSocket::UdpResponse response;
	if (!valid_)
	{
		return response;
	}

	struct sockaddr_storage addr;
	char buf[512]; //TODO this limits the maximum frame size. (do I really need more?)
	socklen_t fromlen;
	struct pollfd fds[1];
	char ipstr[INET_ADDRSTRLEN];

	fromlen = sizeof addr;

	fds[0].fd = socketDescriptor_;
	fds[0].events = POLLIN;

	//As I'm going to create the data with malloc, but I want to wrap the pointer with a 
	//shared_ptr, I need to provide a custom deleter that uses free instead of delete
	std::function<void(const char*)> deleter = charPointerDeleter;

	if (poll(fds, 1, timeout) > 0)
	{
		//Packet received
		response.timestamp = std::chrono::steady_clock::now();
		ssize_t res = ::recvfrom(socketDescriptor_, buf, sizeof buf, 0, (struct sockaddr*)&addr, &fromlen);

		if (res > 0)
		{
			const char* address = inet_ntop(addr.ss_family, &(((struct sockaddr_in*)&addr)->sin_addr), ipstr, sizeof ipstr);
			response.address = address;
			response.port = ntohs(((struct sockaddr_in*)&addr)->sin_port);

			char* data = (char*) malloc(res);
			memcpy(data, buf, res);

			response.data = std::shared_ptr<const char>(data, deleter);
			response.size = res;
		}
	}
	return response;
}

std::vector<UdpSocket::UdpResponse> UdpSocket::recvfrom(int timeout)
{
	std::vector<UdpResponse> result;
	if (!valid_)
	{
		return result;
	}

	struct sockaddr_storage addr;
	char buf[512]; //TODO this limits the maximum frame size. (do I really need more?)
	socklen_t fromlen;
	struct pollfd fds[1];
	char ipstr[INET_ADDRSTRLEN];

	fromlen = sizeof addr;

	fds[0].fd = socketDescriptor_;
	fds[0].events = POLLIN;

	//As I'm going to create the data with malloc, but I want to wrap the pointer with a 
	//shared_ptr, I need to provide a custom deleter that uses free instead of delete
	std::function<void(const char*)> deleter = charPointerDeleter;

	while(poll(fds, 1, timeout) > 0)
	{
		ssize_t res = ::recvfrom(socketDescriptor_, buf, sizeof buf, 0, (struct sockaddr*)&addr, &fromlen);
		if (res < 0)
		{
			continue;
		}
		UdpResponse response;
		response.timestamp = std::chrono::steady_clock::now();
		const char* address = inet_ntop(addr.ss_family, &(((struct sockaddr_in*)&addr)->sin_addr), ipstr, sizeof ipstr);
		response.address = address;
		response.port = ntohs(((struct sockaddr_in*)&addr)->sin_port);

		char* data = (char*) malloc(res);
		memcpy(data, buf, res);
		
		response.data = std::shared_ptr<const char>(data, deleter);
		response.size = res;
		result.push_back(response);
	}
	return result;
}

