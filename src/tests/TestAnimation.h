#ifndef TEST_ANIMATION_H
#define TEST_ANIMATION_H

#include "BaseAnimation.h"

class TestAnimation : public BaseAnimation
{
public:
	TestAnimation();
	~TestAnimation(){}

	virtual AnimationType getAnimType() { return BaseAnimation::AnimationType::SINGLE;}
	std::vector<std::vector<Keyframe>::size_type>& getKeyframePositions(){return keyframePositions_;}
	BaseAnimation::ChannelKeyList& getKeyData(){return data_; }

};

#endif