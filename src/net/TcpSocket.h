#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

/**
 * Tcp socket class wraps a POSIX type tcp socket and provides a basic 
 * API to access to its capabilities.
 * 
 * Supports both client and server modes.
 */

#include "Socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
#include <memory>

class TcpSocket : public Socket
{
public:

	/**
	 * @brief      TcpData wraps a TCP recv data.
	 * The memory is automatically managed, and you can pass this 
	 * struct by copy, as the data is internally shared between all of the copies
	 */
	struct TcpData
	{
		TcpData() : size(0){}
		std::shared_ptr<const char> data;
		size_t size;
		std::chrono::steady_clock::time_point timestamp;
	};

	/**
	 * @brief      Create a new connect mode TCP socket
	 *
	 * @param[in]  ipAddress  The ip address to connect to
	 * @param[in]  port       The port to connect to
	 */
	explicit TcpSocket(std::string ipAddress, uint16_t port);
	
	/**
	 * @brief      Create a new server TCP socket on the specified
	 * port
	 *
	 * @param[in]  port  The port to listen to
	 */
	explicit TcpSocket(uint16_t port);

	/**
	 * @brief      Default constructor for the socket. The socket will
	 * 				remain invalid unless it gets initialized by another TcpSocket
	 */
	TcpSocket();

	/**
	 * @brief      Delete the copy constructor as it make no sense to copy a TcpSocket
	 *
	 * @param[in]  other  The other
	 */
	TcpSocket(const TcpSocket& other) = delete;
	TcpSocket& operator=(const TcpSocket& other) = delete;

	TcpSocket(TcpSocket&& other);
	TcpSocket& operator=(TcpSocket&& other);

	
	~TcpSocket();

	const std::string& getIpAddress() {return ipAddress_;}

	ssize_t send (const std::string& txt);
	ssize_t send (const char* txt);
	ssize_t send (const char* data, size_t len);
	ssize_t send (const uint8_t* data, size_t len);

	void close();
	TcpData recv(int timeout = 2000);//TODO revisit

	bool accept(TcpSocket& socket, int timeout = 2000);

	virtual SocketType getSocketType();
private:

	int socketDescriptor_;
	std::string ipAddress_;
	struct sockaddr_in address_;

};

#endif