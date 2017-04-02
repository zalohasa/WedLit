#include "WedLit.h"
#include "Node.h"
#include "TcpSocket.h"
#include "UdpSocket.h"

#include <cassert>
#include <strings.h>
#include <iostream>
#include <chrono>

INIT_LOG(NODE);

Node::Node(std::string ip, uint16_t tcp_port, uint16_t udp_port, const NodeController::NodeId& nodeId) :
	ip_(ip),
	tcp_port_(tcp_port),
	udp_port_(udp_port),
	nodeId_(nodeId)
	//socket_(ip, port, false)
{
	//assert(socket_.isValid());
	TRACE("New node with ip: {}", ip_);
}

Node::~Node()
{
	//NOthing to do 
}

AnimationChannel Node::getChannel()
{
	return currentCh_;
}

const NodeController::NodeId& Node::getNodeId()
{
	return nodeId_;
}

const std::string& Node::getNodeAddr()
{
	return ip_;
}

bool Node::setChannel(AnimationChannel ch)
{
	currentCh_ = ch;
	TcpSocket s(ip_, tcp_port_);
	char cmd[3];
	bcopy(CMD_CH_CHANNEL, cmd, 2);
	cmd[2] = ch;

	if (s.isValid() && s.send((uint8_t*)cmd, sizeof(cmd)))
	{
		TRACE("Node {} set to channel {}", nodeId_, ch);
		s.recv();
		return true;
	}

	ERROR("Unable to set channel: {} to node {}:{}", ch, nodeId_, ip_);
	return false;
}

bool Node::off()
{
	TcpSocket s(ip_, tcp_port_);
	if (s.isValid() && s.send(CMD_OFF, sizeof(CMD_OFF)))
	{
		s.recv();
		return true;
	}
	ERROR("Unable to off node {}:{}", nodeId_, ip_);
	return false;
}

bool Node::clear()
{
	TcpSocket s(ip_, tcp_port_);
	if (s.isValid() && s.send(CMD_CLEAR, sizeof(CMD_CLEAR)))
	{
		s.recv();
		return true;
	}

	ERROR("Unable to clear node {}:{}", nodeId_, ip_);
	return false;
}

const std::string& Node::getIp()
{
	return ip_;
}

bool Node::sendKeyframesInKeyframeMode(KeyframeData data)
{
	TcpSocket init(ip_, tcp_port_);
	assert(init.isValid());//TODO change for something less intrusive(all asserts here)
	init.send(CMD_INSERT, sizeof(CMD_INSERT));
	init.recv();
	init.close();

	TcpSocket keyData(ip_, tcp_port_);
	assert(keyData.isValid());
	keyData.send(data.getRawData(), data.getSize());
	keyData.recv();
	keyData.close();

	TcpSocket end(ip_, tcp_port_);
	assert(end.isValid());
	end.send(CMD_END, sizeof(CMD_END));
	end.recv();
	end.close();

	return true;
}

bool Node::sendSyncRequest(std::chrono::steady_clock::time_point startPoint, int32_t& offset, int32_t& rt)
{
	using namespace std::chrono;
	using TimePoint = time_point<steady_clock, milliseconds>;
	UdpSocket sk(ip_, udp_port_);
	if (!sk.isValid())
	{
		ERROR("Unable to open UDP socket to {}", ip_);
		return false;
	}
	TimePoint t0 (duration_cast<milliseconds>(steady_clock::now() - startPoint));
	sk.send(CMD_TIME_PING);
	UdpSocket::UdpResponse resp = sk.recvfrom_one(250);
	if (resp.isNull())
	{
		WARN("No response received while sync request to node {}", ip_);
		return false;
	}
	int32_t t1raw = *((uint32_t*)resp.data.get());
	int32_t t2raw = t1raw+1; //Tested empirically that the node takes 1ms from receiving the 
						//frame to send the new one.
	milliseconds d1(t1raw);
	milliseconds d2(t2raw);
	TimePoint t1(d1);
	TimePoint t2(d2);

	TimePoint t3 (duration_cast<milliseconds>(resp.timestamp - startPoint));
	DEBUG("t0: {} t1: {} t2: {} t3: {}", t0.time_since_epoch().count(), 
		t1.time_since_epoch().count(), 
		t2.time_since_epoch().count(),
		t3.time_since_epoch().count());

	offset = duration_cast<milliseconds>((t1 - t0) + (t2 - t3)).count() / 2;
	rt = duration_cast<milliseconds>((t3 - t0) - (t2 - t1)).count();
	return true;
}