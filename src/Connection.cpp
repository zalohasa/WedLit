#include "Connection.h"
#include "Commands.h"
#include "AnimationController.h"

#include <iostream>
#include <sstream>

INIT_LOG(CONNECTION);

Connection::Connection(TcpSocket& conn) :
	socket_(std::move(conn))
{

}

Connection::~Connection()
{

}

void Connection::exec()
{
	DEBUG("Processing connection from {}", socket_.getIpAddress());	
	while (socket_.isValid() && (!appContext.exitReq))
	{
		TcpSocket::TcpData d = socket_.recv(2000);
		if (d.size > 0)
		{
			processCommand(d);
		}
	}
	DEBUG("Connection with {} finished", socket_.getIpAddress());
}

void Connection::processCommand(TcpSocket::TcpData data)
{
	std::string raw(data.data.get(), data.size);
	std::string paramStr;
	std::vector<std::string> params;
	size_t idx =raw.find(":");
	std::string command = raw.substr(0, idx);
	if (idx != std::string::npos)
	{
		paramStr = raw.substr(idx+1);
		params = parseParams(paramStr);
	}

	DEBUG("Command received: {}", command);
	for(auto s : params)
	{
		DEBUG("Param: {}", s);
	}

	if (command == "listanim")
	{
		std::vector<std::string> res = Commands::getAnimationsList();
		std::stringstream ss;
		for (auto s : res)
		{
			ss << s;
			ss << "\n";
		}
		socket_.send(ss.str());
	}
	else if (command == "play")
	{
		if (params.size() == 0)
		{
			ERROR("A parameter is needed to play an animation (File name)");
			return;
		}
		
		Commands::playAnimation(params[0]);
	}
	else if (command == "stop")
	{
		Commands::stop();
	}
	else if (command == "off")
	{
		Commands::off();
	}
	else if (command == "stopoff")
	{
		Commands::stop();
		Commands::off();
	}
	else if (command == "listnodes")
	{
		std::vector<std::string> res = Commands::getNodeList();
		std::stringstream ss;
		for (auto s : res)
		{
			ss << s;
			ss << "\n";
		}
		socket_.send(ss.str());
	}
}

std::vector<std::string> Connection::parseParams(const std::string &rawParams)
{
	std::vector<std::string> res;
	std::string all = rawParams;
	size_t idx;
	while ((idx = all.find(":")) != std::string::npos)
	{
		res.push_back(all.substr(0, idx));
		all = all.substr(idx+1);
	}
	if (all.size() > 0)
	{
		res.push_back(all);
	}

	return res;
}