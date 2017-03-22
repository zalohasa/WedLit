#ifndef KEYFRAME_DATA_H
#define KEYFRAME_DATA_H

#include "BaseAnimation.h"

#include <memory>

class KeyframeData
{
public: 
	enum class DataType {DATA_SINGLE, DATA_MULTI};
	KeyframeData(const BaseAnimation::ChannelKeyList& chKeys);
	KeyframeData(const BaseAnimation::SharedKeyList keyList);
	KeyframeData(const BaseAnimation::KeyList& keyList);
	KeyframeData(const KeyframeData& other);
	KeyframeData& operator=(const KeyframeData& other);
	~KeyframeData();

	const uint8_t* getRawData() const;
	size_t getSize() const;
	size_t getNumberOfChannels() const;

	DataType getDataType();


private:

	size_t numberOfBytes_multi(const BaseAnimation::ChannelKeyList& chKeys, size_t& nChannels);
	void initializeChannelData_multi(const BaseAnimation::ChannelKeyList& chKeys);
	size_t numberOfBytes_single(const BaseAnimation::SharedKeyList keyList);
	void initializeChannelData_single(const BaseAnimation::SharedKeyList keyList);

	std::shared_ptr<uint8_t> data_;
	size_t size_;
	size_t nChannels_;
	DataType type_;
};

#endif