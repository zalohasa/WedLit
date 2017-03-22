#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <cstdint>

//Empty base class just in case I need to add common code to sockets (probably)

class Socket
{
public:
	enum class SocketType {UDP, TCP};
	virtual ~Socket(){}

	virtual SocketType getSocketType() = 0;
	bool isValid(){return valid_;}

protected:
	Socket() : valid_(true){}
	bool valid_;

};

#endif