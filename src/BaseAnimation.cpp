#include <iostream>
#include "BaseAnimation.h"

using keyindex_t = std::vector<Keyframe>::size_type;
static constexpr std::vector<Keyframe>::size_type END = std::numeric_limits<std::vector<Keyframe>::size_type>::max();

BaseAnimation::BaseAnimation() : started_(false)
{
	//Nothing to do yet.
}

BaseAnimation::~BaseAnimation()
{
	//TODO
}

void BaseAnimation::getNKeys(ChannelKeyList &list, size_t keysPerChannel)
{
	if (!list.empty())
	{
		//TODO assert
		std::cout << __FILE__ << __FUNCTION__ << "The list must be empty" << std::endl;
		return;
	}

	size_t channel = 0;
	for(SharedKeyList channelList : data_)
	{
		keyindex_t channelIndex = keyframePositions_[channel];
		if (channelIndex != END)
		{
			//There are keyframes for this channel yet.
			size_t remaining = channelList->size() - channelIndex;
			if (started_)
			{
				--remaining;
			}
			size_t numberOfFrames = keysPerChannel;

			if (remaining < keysPerChannel)
			{
				//Not enough frames to fill the vector with keysPerChannel keyframes
				numberOfFrames = remaining;
			}
			//std::cout << "Channel: " << channel << " remaining: " << remaining << " frames: " << numberOfFrames << std::endl;
			SharedKeyList keyframes = std::make_shared<KeyList>(numberOfFrames);

			// if (started_)
			// {
			// 	//If the animation is ongoing, the channelIndex represents the last keyframe
			// 	//sent.
			// 	//Buf if the animation is starting now, the channelIndex represents nothing, 
			// 	//and we need to avoid to increment it before starting to send the keframes.
			// 	++channelIndex;
			// }
			bool notFirstTime = started_;

			for (keyindex_t dstIndex = 0; dstIndex < numberOfFrames; ++dstIndex)
			{
				++channelIndex;
				if (!notFirstTime)
				{
					--channelIndex;
					notFirstTime = true;
				}
				//std::cout << "Copy for channel " << channel << " Index: " << channelIndex << std::endl;

				(*keyframes)[dstIndex] = (*channelList)[channelIndex];
			}

			//Copy finished. Flag the last keyframe sent for current channel, or END
			//if there are no more keyframes.
			if (channelIndex < (channelList->size() - 1))
			{
				//Still pending frames.
				keyframePositions_[channel] = channelIndex;
			}
			else
			{
				//End of the vector reached, flag it as END
				keyframePositions_[channel] = END;
			}
			
			list.push_back(keyframes);
		}
		else
		{
			//Add an empty vector, no more keyframes for this channel.
			list.push_back(std::make_shared<KeyList>(0));
		}
		++channel;
	}
	started_ = true;

}

size_t BaseAnimation::getChannels() const
{
	return data_.size();
}

void BaseAnimation::init()
{
	keyframePositions_.resize(data_.size());
	reset();
}

void BaseAnimation::reset()
{
	for (size_t i = 0; i < keyframePositions_.size(); ++i)
	{
		keyframePositions_[i] = 0;
	}
	started_ = false;
}

keytime_t BaseAnimation::getFirstKeyout()
{
	if (!started_)
	{
		return 0;
	}

	keytime_t time = KEYTIME_MAX;
	size_t channel = 0;
	for (SharedKeyList channelList : data_)
	{
		if (keyframePositions_[channel] != END)
		{
			if ((channelList->at(keyframePositions_[channel])).time < time)
			{
				time = (channelList->at(keyframePositions_[channel])).time;
			}
		}
		++channel;
	}

	return time;
}