#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "Socket.h"

#include <sys/socket.h>
#include <netinet/in.h>

class TcpSocket : public Socket
{
public:

	TcpSocket(std::string ipAddress, uint16_t port);
	~TcpSocket();

	ssize_t send (const std::string& txt);
	ssize_t send (const char* txt);
	ssize_t send (const char* data, size_t len);
	ssize_t send (const uint8_t* data, size_t len);

	void close();
	void reopen();
	void recv();//TODO revisit

	//TODO make recv methods

	virtual SocketType getSocketType();
private:

	int socketDescriptor_;
	struct sockaddr_in address_;

};

#endif