#include "TcpSocket.h"
#include "WedLit.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/time.h>

INIT_LOG(TCP_SOCKET);

static void charPointerDeleter(const char* pointer)
{
	free((void*) pointer);
}

TcpSocket::TcpSocket() : Socket()
{
	valid_ = false;
	socketDescriptor_ = -1;
}

TcpSocket::TcpSocket(std::string ipAddress, uint16_t port) : Socket()
{
	socketDescriptor_ = socket(AF_INET, SOCK_STREAM, 0);
	if (socketDescriptor_ < 0)
	{
		ERROR("Error: ", strerror(errno));
		valid_ = false;
	}
	else
	{
		ipAddress_ = ipAddress;
		address_.sin_family = AF_INET;
		inet_aton(ipAddress.c_str(), &(address_.sin_addr));
		address_.sin_port = htons(port);

		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;

		//Setting send and receive timeout to 3 seconds, to avoid hanging forever
		if (setsockopt(socketDescriptor_, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof(struct timeval)) < 0)
		{
			ERROR("Error setting option: {}", strerror(errno));
		}

		if (setsockopt(socketDescriptor_, SOL_SOCKET, SO_SNDTIMEO, (const void*)&tv, sizeof(struct timeval)) < 0)
		{
			ERROR("Error setting option: {}", strerror(errno));
		}

		if (connect(socketDescriptor_, (struct sockaddr*) &address_, sizeof(address_)) < 0)
		{
			//Did the connection time out?
			if (errno == EINPROGRESS)
			{
				WARN("Connection timed out, retrying...");
				if (connect(socketDescriptor_, (struct sockaddr*) &address_, sizeof(address_)) < 0)
				{
					ERROR("Error connecting to {}:{} {}", ipAddress, port, strerror(errno));
					valid_ = false;
				}
			}
			else 
			{
				ERROR("Error connecting to {}:{} {}", ipAddress, port, strerror(errno));
				valid_ = false;
			}
		}
	}
}

TcpSocket::TcpSocket(uint16_t port) : Socket()
{
	socketDescriptor_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socketDescriptor_ < 0)
	{
		ERROR("Error: ", strerror(errno));
		valid_ = false;
	}
	else
	{
		bzero((char*) &address_, sizeof(address_));
		address_.sin_family = AF_INET;
		address_.sin_addr.s_addr = INADDR_ANY;
		address_.sin_port = htons(port);
		if (::bind(socketDescriptor_, (struct sockaddr*) &address_, sizeof(address_)) < 0)
		{
			ERROR("Error binding socket to port {}: {}", port, strerror(errno));
			valid_ = false;
		}
		else
		{
			if(::listen(socketDescriptor_, MAX_SERVER_CONNECTIONS) < 0)
			{
				ERROR("Error listening on port {} {}", port, strerror(errno));
				valid_ = false;
			}
		}
	}
}

TcpSocket::TcpSocket(TcpSocket&& other) : Socket()
{
	socketDescriptor_ = other.socketDescriptor_;
	valid_ = other.valid_;
	ipAddress_ = other.ipAddress_;

	//Invalidate the other socket.
	other.socketDescriptor_ = -1;
	other.valid_ = false;
	other.ipAddress_.clear();
}

TcpSocket& TcpSocket::operator=(TcpSocket&& other)
{
	this->socketDescriptor_ = other.socketDescriptor_;
	this->valid_ = other.valid_;
	this->ipAddress_ = other.ipAddress_;

	other.socketDescriptor_ = -1;
	other.valid_ = false;
	other.ipAddress_.clear();

	return *this;
}

TcpSocket::~TcpSocket()
{
	if (valid_)
	{
		::shutdown(socketDescriptor_, SHUT_RDWR);
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
		TRACE("Socket not valid, exiting");
		return data;
	}
	
	struct pollfd fds[1];
	fds[0].fd = socketDescriptor_;
	fds[0].events = POLLIN | POLLHUP;

	int ret = poll(fds, 1, timeout);

	if (ret > 0)
	{
		if (fds[0].revents & POLLIN)
		{
			data.timestamp = std::chrono::steady_clock::now();
			char buff[512];
			ssize_t res = ::recv(socketDescriptor_, (void*)buff, sizeof(buff), 0);
			if (res > 0)
			{
				TRACE("Readed {} bytes", res);
				char* raw = (char*) malloc(res);
				memcpy(raw, buff, res);
				std::function<void(const char*)> deleter = charPointerDeleter;
				data.data = std::shared_ptr<const char>(raw, deleter);
				data.size = res;
			}
			else if (res == 0)
			{
				//The peer has performed an orderly shutdown
				TRACE("Readed 0 bytes, closing socket");
				close();
			}
			else
			{
				ERROR("Error while recv: {}", strerror(errno));
			}
		}
		if (fds[0].revents & POLLHUP)
		{
			TRACE("Remote {} connection closed", ipAddress_);
			valid_ = false;
		}
	}
	else if (ret < 0)
	{
		WARN("Error while polling {}", strerror(errno));
		valid_ = false;
	}
	else
	{
		//TRACE("Poll timed out");
	}
	return data;
}

bool TcpSocket::accept(TcpSocket& socket, int timeout)
{
	if (socket.isValid())
	{
		ERROR("You can not use a valid TCP socket here. You need to use a default constructed TcpSocket");
		return false;
	}

	struct sockaddr_in addr;
	socklen_t addlen = sizeof(addr);
	struct pollfd fds[1];
	fds[0].fd = socketDescriptor_;
	fds[0].events = POLLIN;

	if (poll(fds, 1, timeout) > 0)
	{
		if ((socket.socketDescriptor_ = ::accept(socketDescriptor_, (struct sockaddr*) &(addr), &addlen)) < 0)
		{
			ERROR("Error while accepting the connection: {}", strerror(errno));
			return false;
		}
		else
		{
			char ipstr[INET_ADDRSTRLEN];
			const char* address = inet_ntop(addr.sin_family, &(addr.sin_addr), ipstr, sizeof(ipstr));
			socket.ipAddress_ = address;
			socket.valid_ = true;
			return true;
		}
	}

	return false;
}

void TcpSocket::close()
{
	if (valid_)
	{
		::close(socketDescriptor_);
		valid_ = false;
	}
}