#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "Socket.h"

#include <memory>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>

class UdpSocket : public Socket
{
public:
	struct UdpResponse
	{
		std::string address;
		uint16_t port;
		std::shared_ptr<const char> data;
		size_t size;
		std::chrono::steady_clock::time_point timestamp;

		bool isNull(){return address.empty();}
	};

	UdpSocket(std::string ipAddress, uint16_t port, bool broadcast = false);
	~UdpSocket();

	void send(const std::string& txt);
	void send(const char* txt);
	void send(const uint8_t* data, size_t len);
	UdpResponse recvfrom_one(int timeout);
	std::vector<UdpResponse> recvfrom(int timeout);

	virtual SocketType getSocketType();

private:
	int socketDescriptor_;
	struct sockaddr_in address_;
};

#endif