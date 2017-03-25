#include "Server.h"
#include "TcpSocket.h"
#include "Connection.h"

INIT_LOG(SERVER);

void Server::manageConnection(TcpSocket* connection, std::atomic<bool>* done)
{
	Connection c(*connection);
	c.exec();
	*done = true;
	DEBUG("Connection thread finished");
}

Server::~Server()
{
	for (Worker& w : workers)
	{
		if (w.thread.joinable())
		{
			DEBUG("Waiting for connection thread to finish");
			w.thread.join();
		}
	}
}

Server::Server(uint16_t listenPort) : 
	srvSocket_(listenPort)
{
	if (srvSocket_.isValid())
	{
		INFO("Server listeinig in port {}", listenPort);
	}
	else
	{
		ERROR("Unable to create server in port {}", listenPort);
	}
}

void Server::exec()
{
	if (srvSocket_.isValid())
	{
		TcpSocket conn;
		while (!appContext.exitReq)
		{
			TcpSocket tmpConn;
			if (srvSocket_.accept(tmpConn, -1))
			{
				bool found = false;
				for (Worker& w : workers)
				{
					if (!w.thread.joinable() || (w.thread.joinable() && w.done))
					{
						if (w.thread.joinable())
						{
							//If i'm here, the thread is finished, so join it to cleanup.
							w.thread.join();
						}
						TRACE("New connection detected, initializing new thread");
						conn = std::move(tmpConn);
						w.done = false;
						w.thread = std::thread(&Server::manageConnection, &conn, &(w.done));
						found = true;
						break;
					}
				}
				if (!found)
				{
					WARN("Rejecting new connection as there are no more slots available");
					tmpConn.close();
				}
			}
		}
	}
	else
	{
		ERROR("Can't start server main loop, no valid server socket");
	}
}