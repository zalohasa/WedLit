#ifndef BASE_ANIMATION_H
#define BASE_ANIMATION_H

#include <vector>
#include <list>
#include <memory>
#include <unordered_map>

#include "BasicTypes.h"

class BaseAnimation
{
public:

	enum class AnimationType {SINGLE, CYCLE};
	
	using KeyList = std::vector<Keyframe>;
	using SharedKeyList = std::shared_ptr<KeyList>;
	using ChannelKeyList = std::vector<SharedKeyList>;
	using AnimationParameters = std::unordered_map<std::string, std::string>;


	explicit BaseAnimation();
	virtual ~BaseAnimation();
	
	size_t getChannels() const;
	void getNKeys(ChannelKeyList& list, size_t keysPerChannel);
	void reset();
	virtual AnimationType getAnimType() = 0;
	keytime_t getFirstKeyout();

protected:
	void init();
	//virtual void fillChannelData(ChannelKeyList& dataList) = 0; //If the different animations load the keyframes on construction time, this is not needed.
	bool started_;
	std::vector<std::vector<Keyframe>::size_type> keyframePositions_;
	ChannelKeyList data_;

};

#endif //BASE_ANIMATION_H