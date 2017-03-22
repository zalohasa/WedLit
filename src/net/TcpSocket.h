#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "Socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
#include <memory>

class TcpSocket : public Socket
{
public:

	struct TcpData
	{
		std::shared_ptr<const char> data;
		size_t size;
		std::chrono::steady_clock::time_point timestamp;
	};

	TcpSocket(std::string ipAddress, uint16_t port);
	~TcpSocket();

	ssize_t send (const std::string& txt);
	ssize_t send (const char* txt);
	ssize_t send (const char* data, size_t len);
	ssize_t send (const uint8_t* data, size_t len);

	void close();
	void reopen();
	TcpData recv(int timeout = 2000);//TODO revisit

	//Server methods
	bool bind(uint16_t port);
	bool listen(int max_connexions);

	//TODO make recv methods

	virtual SocketType getSocketType();
private:

	int socketDescriptor_;
	struct sockaddr_in address_;

};

#endif