#include "KeyframeData.h"
#include "BasicTypes.h"

static void uint8_t_pointerDeleter(uint8_t* data)
{
	free((void*) data);
}

KeyframeData::KeyframeData(const BaseAnimation::ChannelKeyList& chKeys) :
	size_(0),
	nChannels_(0)
{
	size_ = numberOfBytes_multi(chKeys, nChannels_);
	uint8_t* rawData = (uint8_t*)malloc(sizeof(uint8_t) * size_);

	std::function<void(uint8_t*)> deleter = uint8_t_pointerDeleter;
	data_ = std::shared_ptr<uint8_t>(rawData, deleter);
	initializeChannelData_multi(chKeys);
	type_ = DataType::DATA_MULTI;
}

KeyframeData::KeyframeData(const BaseAnimation::KeyList& keylist)
{
	
}

KeyframeData::KeyframeData(const BaseAnimation::SharedKeyList keyList) :
	size_(0),
	nChannels_(0)
{
	size_ = numberOfBytes_single(keyList);
	uint8_t* rawData = (uint8_t*)malloc(sizeof(uint8_t) * size_);

	std::function<void(uint8_t*)> deleter = uint8_t_pointerDeleter;
	data_ = std::shared_ptr<uint8_t>(rawData, deleter);
	initializeChannelData_single(keyList);
	nChannels_ = 1;
	type_ = DataType::DATA_SINGLE;
}

KeyframeData::KeyframeData(const KeyframeData& other) : 
	data_(other.data_),
	size_(other.size_),
	nChannels_(other.nChannels_)
{

}

KeyframeData& KeyframeData::operator=(const KeyframeData &other)
{
	data_.reset();
	data_ = other.data_;
	size_ = other.size_;
	nChannels_ = other.nChannels_;
	return *this;
}

KeyframeData::~KeyframeData()
{
	
}

const uint8_t* KeyframeData::getRawData() const
{
	return data_.get();
}

size_t KeyframeData::getSize() const
{
	return size_;
}

size_t KeyframeData::getNumberOfChannels() const
{
	return nChannels_;
}

KeyframeData::DataType KeyframeData::getDataType()
{
	return type_;
}

size_t KeyframeData::numberOfBytes_multi(const BaseAnimation::ChannelKeyList& chKeys, size_t& nChannels)
{
	uint32_t nKeys = 0;
	nChannels = 0;
	size_t size = 0;

	for (auto ch : chKeys)
	{
		++nChannels;
		nKeys += ch->size();
	}

	size = ((sizeof(KeyIndex) * nChannels) + (sizeof(Keyframe) * nKeys));
	return size;
}

size_t KeyframeData::numberOfBytes_single(const BaseAnimation::SharedKeyList keyList)
{
	return (keyList->size() * sizeof(Keyframe)) + 1;
}

void KeyframeData::initializeChannelData_single(const BaseAnimation::SharedKeyList keyList)
{
	data_.get()[0] = keyList->size();
	Keyframe* keyData = reinterpret_cast<Keyframe*>(&(data_.get())[1]);
	size_t keyIndex = 0;
	for (auto key = keyList->cbegin(); key != keyList->cend(); ++key)
	{
		keyData[keyIndex] = *key;
		++keyIndex;
	}
}

void KeyframeData::initializeChannelData_multi(const BaseAnimation::ChannelKeyList& chKeys)
{
	uint16_t startKeysIndex = sizeof(KeyIndex) * nChannels_;
	KeyIndex* chData = reinterpret_cast<KeyIndex*>(data_.get());
	Keyframe* keyData = reinterpret_cast<Keyframe*>(&(data_.get())[startKeysIndex]);
	uint8_t chNumber = 0;
	size_t keyIndex = 0;
	for (auto ch : chKeys)
	{
		chData[chNumber].nKeys = ch->size();
		chData[chNumber].chId = chNumber;
		chData[chNumber].keys_offset = (sizeof(Keyframe) * keyIndex) + startKeysIndex;
		for (auto it = ch->cbegin(); it != ch->cend(); ++it)
		{
			keyData[keyIndex] = *it;
			++keyIndex;
		}
		++chNumber;
	}
}