#include "WedLit.h"
#include "NodeController.h"
#include "Node.h"
#include "KeyframeData.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <unistd.h>
#include <unordered_map>

INIT_LOG(NODE_CONTROLLER);

static long getTimestamp()
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	std::cout << "Time: " << time.tv_nsec << std::endl;
	return (time.tv_sec * 1000) + (time.tv_nsec / (1000*1000));
}

void NodeController::discovery_thread_entry(NodeController* nc)
{
	nc->internal_discovery();
}

NodeController::NodeController(const std::string& broadcastIp, uint16_t broadcastPort) : 
socket_(broadcastIp, broadcastPort, true),
discoveryTimeout_(250)
{
	assert(socket_.isValid());
}

NodeController::~NodeController()
{
	if (discoveryThread_.joinable())
	{
		DEBUG("Waiting for discovery thread to finish");
		discoveryThread_.join();
	}
}

void NodeController::setChannelMap(const NodeController::ChannelMap& map)
{
	channelMap_ = map;
	for (auto ch : map)
	{
		if (nodeIdToNode_.find(ch.first) == nodeIdToNode_.end())
		{
			WARN("NodeID not found in node list");
			continue;
		}
		else
		{
			nodeIdToNode_[ch.first]->setChannel(ch.second);
		}
	}
	remakeChannelToNodes();
}

void NodeController::setAllToChannel(AnimationChannel channel)
{
	DEBUG("Setting all nodes to channel: {}", channel);
	channelMap_.clear();
	for (auto ch : nodeIdToNode_)
	{
		ch.second->setChannel(channel);
		channelMap_.insert(std::make_pair(ch.first, channel));
	}
	remakeChannelToNodes();
}

void NodeController::setChannelAuto(size_t numOfChannels)
{
	DEBUG("Setting all nodes to autochannel");
	channelMap_.clear();
	size_t idx = 0;
	for (auto ch : nodeIdToNode_)
	{
		AnimationChannel dstCh = idx % numOfChannels;
		ch.second->setChannel(dstCh);
		channelMap_.insert(std::make_pair(ch.first, dstCh));
		++idx;
	}
	remakeChannelToNodes();
}

void NodeController::setChannel(const NodeId &nodeId, AnimationChannel channel)
{
	DEBUG("Setting channel {} to node {}", channel, nodeId);
	if (channelMap_.find(nodeId) != channelMap_.end())
	{
		channelMap_[nodeId] = channel;

	}
	else
	{
		WARN("No such nodeid in channelmap"); 
	}

	if (nodeIdToNode_.find(nodeId) != nodeIdToNode_.end())
	{
		nodeIdToNode_[nodeId]->setChannel(channel);
	}
	else
	{
		WARN("No such nodeId in nodemap"); 
	}
	remakeChannelToNodes();
}

void NodeController::off()
{
	DEBUG("Sending broadcast OFF");
	socket_.send(CMD_OFF);
}

void NodeController::off(const NodeId& nodeId)
{
	DEBUG("Sending off to node {}", nodeId);
	if (nodeIdToNode_.find(nodeId) != nodeIdToNode_.end())
	{
		nodeIdToNode_[nodeId]->off();
	}
	else
	{
		DEBUG("No such nodeId in nodemap");
	}
}

void NodeController::play()
{
	DEBUG("Sending broadcast play");
	socket_.send(CMD_PLAY);
}

void NodeController::cycle()
{
	DEBUG("Sending broadcast cycle");
	socket_.send(CMD_CYCLE);
}

void NodeController::stop()
{
	DEBUG("Sending broadcast stop");
	socket_.send(CMD_STOP);
}

void NodeController::clear()
{
	DEBUG("Sending clear to nodes");
	for (auto nodePair : nodeIdToNode_)
	{
		//socket_.send(CMD_CLEAR);
		TRACE("Sending clear to node: {}", nodePair.first);
		nodePair.second->clear();
	}
}

std::vector<std::string> NodeController::getNodesAddr() const
{
	std::vector<std::string> ips;
	ips.reserve(nodeIdToNode_.size());

	for (auto node : nodeIdToNode_)
	{
		ips.push_back(node.second->getNodeAddr());
	}
	return ips;

}

std::vector<NodeController::NodeId> NodeController::getNodes() const
{
	std::vector<NodeId> nodes;
	nodes.reserve(nodeIdToNode_.size());
	
	for (auto ch : nodeIdToNode_)
	{
		nodes.push_back(ch.first);
	}
	return nodes;
}

size_t NodeController::getNodeCount() const
{
	return nodeIdToNode_.size();
}

void NodeController::setDiscoveryTimeout(int newTimeout)
{
	discoveryTimeout_ = newTimeout;
}

void NodeController::sendKeys(const BaseAnimation::ChannelKeyList &keys)
{
	//TODO Split key in various frames if keycount is big.
	//TODO do that work in a different thread.
	//TODO validate no less channels in the animation than in the channelMap.
	KeyframeData kData(keys);
	DEBUG("Sending {} bytes over UDP", kData.getSize());
	socket_.send(kData.getRawData(), kData.getSize());
}

void NodeController::sendInitialKeys(const BaseAnimation::ChannelKeyList &keys)
{
	for (int chNum = 0; chNum < keys.size(); ++chNum)
	{
		BaseAnimation::SharedKeyList allKeys = keys[chNum];
		if (allKeys->empty())
		{
			DEBUG("No keys for channel {}", chNum);
			continue;
		}

		if (allKeys->size() < MAX_KEYS_PER_PACKET_INIT_MODE)
		{
			DEBUG("Sending all keys to channel {} - Keys: {}", chNum, allKeys->size());
			KeyframeData data(allKeys);
			putChannelInKeyIn(chNum);
			sendToChannel(data, chNum);
			exitChannelFromKeyIn(chNum);
		}
		else
		{
			DEBUG("Splitting keys in many frames for channel {}", chNum);
			putChannelInKeyIn(chNum);
			size_t numOfKeys = allKeys->size();
			size_t lastSent = 0;
			//Fill n times the full frame
			for (int i = 0; i < (allKeys->size() / MAX_KEYS_PER_PACKET_INIT_MODE); ++i)
			{
				BaseAnimation::SharedKeyList keyList = 
					std::make_shared<BaseAnimation::KeyList>(allKeys->begin() + lastSent, allKeys->begin() + lastSent + MAX_KEYS_PER_PACKET_INIT_MODE);
				TRACE("Sending from {} to {} Channel: {} - Keys: {}", lastSent, lastSent + MAX_KEYS_PER_PACKET_INIT_MODE, chNum, keyList->size());
				KeyframeData data(keyList);
				sendToChannel(data, chNum);
				lastSent = lastSent + MAX_KEYS_PER_PACKET_INIT_MODE;
			}

			//Send the last packet with remaining frames.
			if ((allKeys->size() % MAX_KEYS_PER_PACKET_INIT_MODE) > 0)
			{
				BaseAnimation::SharedKeyList keyList = 
						std::make_shared<BaseAnimation::KeyList>(allKeys->begin() + lastSent, allKeys->end());
					TRACE("Sending last packet from {} to {} Channel: {} - Keys: {}", lastSent, numOfKeys-1, chNum, keyList->size());
				KeyframeData data(keyList);
				sendToChannel(data, chNum);
			}
			exitChannelFromKeyIn(chNum);
		}
	}
}

void NodeController::init()
{
	discoverAllNodes();
}

void NodeController::startDiscoveryThread()
{
	INFO("Starting discovery thread for new nodes");
	discoveryThread_ = std::thread(&NodeController::discovery_thread_entry, this);
}

void NodeController::discoverAllNodes()
{
	//Lets discover the nodes
	INFO("Finding connected nodes...");
	channelMap_.clear();
	//TODO Change the 4 iterations to some configurable value.
	//Try to discover nodes using 4 discovery rounds.
	for (int i = 0; i < 4; ++i)
	{
		DEBUG("Discovery round {}", i+1);
		socket_.send(CMD_DISCOVERY);
		std::vector<UdpSocket::UdpResponse> response = socket_.recvfrom(discoveryTimeout_);
		for (auto n : response)
		{
			NodeId id = stripNodeID(std::string(n.data.get(),n.size));
			if (nodeIdToNode_.find(id) == nodeIdToNode_.end())
			{
				std::shared_ptr<Node> newNode = std::make_shared<Node>(n.address, 1235, n.port, id);
				newNode->setChannel(0);
				channelMap_.insert(std::make_pair(id, 0));
				nodeIdToNode_.insert(std::make_pair(id, newNode));
				DEBUG("Node found: {} Address: {}", id, n.address);
			}
			else
			{
				DEBUG("Discovered node {} already in node list.", id);
			}
		}
	}
	INFO("Found {} connected nodes", nodeIdToNode_.size());
}

void NodeController::internal_discovery()
{
	UdpSocket discoverySocket("10.0.4.255", 1236, true, false, true);
	assert(discoverySocket.isValid());
	while (discoverySocket.isValid() && (!appContext.exitReq))
	{
		std::vector<UdpSocket::UdpResponse> response = discoverySocket.recvfrom(2000);
		for (auto n : response)
		{
			std::string data(n.data.get(), n.size);
			std::string::size_type idx;
			if ((idx = data.find("-")) != std::string::npos)
			{
				std::string bgn = data.substr(0, idx);
				if (bgn == "lighter")
				{
					NodeId id = stripNodeID(data);
					if (nodeIdToNode_.find(id) == nodeIdToNode_.end())
					{
						std::shared_ptr<Node> newNode = std::make_shared<Node>(n.address, 1235, n.port, id);
						newNode->setChannel(0);
						channelMap_.insert(std::make_pair(id, 0));
						nodeIdToNode_.insert(std::make_pair(id, newNode));
						INFO("New node appeared: {} Address: {}", id, n.address);
					}
					else
					{
						DEBUG("Discovered node {} already in node list.", id);
					}
				}
			}
		}
	}
	DEBUG("Discovery thread finished");

}

NodeController::NodeId NodeController::stripNodeID(std::string nodeIdentification)
{
	std::string::size_type index = nodeIdentification.find("-");
	NodeId nodeId = nodeIdentification.substr(index+1);
	return nodeId;
}

void NodeController::remakeChannelToNodes()
{
	channelToNodes_.clear();
	for (auto ch : channelMap_)
	{
		NodeId id = ch.first;
		if (nodeIdToNode_.find(id) != nodeIdToNode_.end())
		{
			AnimationChannel channel = ch.second;
			if (channelToNodes_.find(channel) == channelToNodes_.end())
			{
				channelToNodes_.insert(
				std::make_pair(ch.second, std::list<std::shared_ptr<Node>>()));
				channelToNodes_.at(channel).push_back(nodeIdToNode_.at(id));
			}
			else
			{
				channelToNodes_.at(channel).push_back(nodeIdToNode_.at(id));
			}
		}
		else
		{
			WARN("The nodeId {} was not found in current node list.", id);
		}
		
	}
}

void NodeController::sendToChannel(KeyframeData& data, AnimationChannel ch)
{
	DEBUG("Sending data to channel {} - {} bytes", ch, data.getSize());
	if (channelToNodes_.find(ch) == channelToNodes_.end())
	{
		WARN("No nodes for channel {}", ch);
		return;
	}
	
	for (auto node : channelToNodes_.at(ch))
	{
		TRACE("Sending to node {}", node->getNodeId());
		node->sendKeyframesInKeyframeMode(data);
	}
}

void NodeController::putChannelInKeyIn(AnimationChannel ch)
{
	if (channelToNodes_.find(ch) == channelToNodes_.end())
	{
		WARN("No nodes for channel {}", ch);
		return;
	}
	
	for (auto node : channelToNodes_.at(ch))
	{
		TRACE("Putting node in KeyIn {}", node->getNodeId());
		node->putNodeInKeyIn();
	}
}

void NodeController::exitChannelFromKeyIn(AnimationChannel ch)
{
	if (channelToNodes_.find(ch) == channelToNodes_.end())
	{
		WARN("No nodes for channel {}", ch);
		return;
	}
	
	for (auto node : channelToNodes_.at(ch))
	{
		TRACE("Exit node from KeyIn {}", node->getNodeId());
		node->exitNodeFromKeyIn();
	}
}

void NodeController::correctDrift(std::chrono::steady_clock::time_point startTime)
{
	//Take the first node of the list.
	std::shared_ptr<Node> node;
	node = (*nodeIdToNode_.begin()).second;

	INFO("Correcting drift using node {} and start {}", node->getNodeId(), startTime.time_since_epoch().count());
	std::vector<std::pair<uint32_t, uint32_t>> data;
	int32_t offset, roundTrip;
	for (int i = 0; i < 4; ++i)
	{
		if (node->sendSyncRequest(startTime, offset, roundTrip))
		{
			DEBUG("offset: {} - Roundtrip: {}", offset, roundTrip);
			data.push_back(std::make_pair(offset, roundTrip));
		}
		else
		{
			WARN("Ignoring syncRequest to node {}", node->getNodeId());
		}
	}

	//Process the data
	//This is a very simple algorithm that takes the pair with minor roundtrip
	//and aplies its offset.
	int32_t minRound = std::numeric_limits<int32_t>::max();
	std::pair<int32_t, int32_t> selected;
	for (auto p : data)
	{
		if (p.second < minRound)
		{
			selected = p;
			minRound = p.second;
		}
	}
	int32_t finalOffset = selected.first;
	INFO("Final offset for node {}, with roundtrip {}, {}ms", node->getNodeId(), minRound, finalOffset*(-1));
	
	 //TODO as the time sync is much more precise than expected, 
	//May be I can delete this if and allways apply the offset.
	if (std::abs(finalOffset) > 10)
	{
		uint8_t data[6];
		data[0] = 's';
		data[1] = 's';
		finalOffset *= (-1);//Reverse the sign of the offset
		memcpy(&data[2], &finalOffset, sizeof(int32_t));
		socket_.send(data, sizeof(data));
	}
}
