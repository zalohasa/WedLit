#ifndef CONNECTION_H
#define CONNECTION_H

#include "WedLit.h"
#include "TcpSocket.h"


class Connection
{
public:
	Connection(TcpSocket& conn);
	~Connection();

	void exec();

private:
	std::vector<std::string> parseParams(const std::string& rawParams);
	void processCommand(TcpSocket::TcpData data);
	TcpSocket socket_;

};

#endif 