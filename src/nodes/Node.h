#ifndef NODE_H
#define NODE_H

#include "BasicTypes.h"
#include "UdpSocket.h"
#include "NodeController.h"
#include "KeyframeData.h"

class Node
{
public:

	Node(std::string ip, uint16_t tcp_port, uint16_t udp_port, const NodeController::NodeId& nodeId);
	virtual ~Node();


	bool setChannel(AnimationChannel ch);
	AnimationChannel getChannel();
	const NodeController::NodeId& getNodeId();
	const std::string& getNodeAddr();

	bool off();
	bool clear();
	bool sendKeyframesInKeyframeMode(KeyframeData data);
 	const std::string& getIp();
 	bool sendSyncRequest(std::chrono::steady_clock::time_point startPoint, int32_t& offset, int32_t& rt);

private:
	std::string ip_;
	uint16_t tcp_port_;
	uint16_t udp_port_;
	NodeController::NodeId nodeId_;
	AnimationChannel currentCh_;
	//UdpSocket socket_; //TODO

};

#endif