#ifndef SERVER_H
#define SERVER_H

#include "WedLit.h"
#include "TcpSocket.h"


class Server
{
public:
	Server(uint16_t listenPort);
	~Server();

	void exec();

private:
	struct Worker
	{
		std::thread thread;
		std::atomic<bool> done;
	};
	static void manageConnection(TcpSocket* connection, std::atomic<bool>* done);
	TcpSocket srvSocket_;
	Worker workers[MAX_SERVER_CONNECTIONS];
};

#endif
