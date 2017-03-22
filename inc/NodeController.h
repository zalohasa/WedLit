#ifndef NODE_CONTROLLER_H
#define NODE_CONTROLLER_H

#include "BaseAnimation.h"
#include "BasicTypes.h"
#include "UdpSocket.h"

#include <string>
#include <unordered_map>
#include <chrono>

static constexpr const char CMD_INSERT[] = "in";
static constexpr const char CMD_CLEAR[] = "cl";
static constexpr const char CMD_CYCLE[] = "cy";
static constexpr const char CMD_END[] = "en";
static constexpr const char CMD_STOP[] = "sp";
static constexpr const char CMD_PLAY[] = "pl";
static constexpr const char CMD_OFF[] = "of";
static constexpr const char CMD_DISCOVERY[] = "dd";
static constexpr const char CMD_DEC_DRIFT[] = "du";
static constexpr const char CMD_INC_DRIFT[] = "ud";
static constexpr const char CMD_SET_DRIFT[] = "ss";//plus uint32_t with the drift
static constexpr const char CMD_ELAPSED[] = "et";
static constexpr const char CMD_PRINT[] = "pr";
static constexpr const char CMD_CH_CHANNEL[] = "ch";
static constexpr const char CMD_TIME_PING[] = "tp";

class Node;
class KeyframeData;

class NodeController
{
public:

	using NodeId = std::string;
	using ChannelMap = std::unordered_map<NodeId, AnimationChannel>;

	NodeController(const std::string& broadcastIp, uint16_t broadcastPort);
	virtual ~NodeController();

	void setChannelMap(const ChannelMap& channelMap);
	void setChannel(const NodeId& nodeId, AnimationChannel channel);
	void setChannelAuto(size_t numOfChannels);
	void setAllToChannel(AnimationChannel channel);

	void off();
	void off(const NodeId& nodeId);
	void play();
	void cycle();
	void stop();
	void clear();
	std::vector<NodeId> getNodes() const;
	size_t getNodeCount() const; //TODO needed?
	void setDiscoveryTimeout(int newTimeout);

	void sendKeys(const BaseAnimation::ChannelKeyList& keys);
	void sendInitialKeys(const BaseAnimation::ChannelKeyList& keys);

	void correctDrift(std::chrono::steady_clock::time_point startTime);

	void init();

protected:
	NodeId stripNodeID(std::string nodeIdentification);
	void remakeChannelToNodes();
	void sendToChannel(KeyframeData& data, AnimationChannel ch);
	std::unordered_map<NodeId, std::shared_ptr<Node>> nodeIdToNode_;
	std::unordered_map<AnimationChannel, std::list<std::shared_ptr<Node>>>channelToNodes_;
	ChannelMap channelMap_;
	UdpSocket socket_;
	int discoveryTimeout_;

private:
	
};

#endif