#ifndef CIRCLE_ANIMATION_H
#define CIRCLE_ANIMATION_H

#include "BaseAnimation.h"

class CircleAnimation : public BaseAnimation
{
public:
	CircleAnimation(const AnimationParameters& params);
	~CircleAnimation(){}

	virtual AnimationType getAnimType() {return BaseAnimation::AnimationType::CYCLE; }
private:
	keytime_t interval_;
	size_t nChannels_;
	uint8_t level_;

};

#endif