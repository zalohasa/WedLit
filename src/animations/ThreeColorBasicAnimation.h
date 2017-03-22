#ifndef THREE_COLOR_BASIC_ANIMATION_H
#define THREE_COLOR_BASIC_ANIMATION_H

#include "BaseAnimation.h"

class ThreeColorBasicAnimation : public BaseAnimation
{
public:

	ThreeColorBasicAnimation(const AnimationParameters& params);
	~ThreeColorBasicAnimation(){};

	virtual AnimationType getAnimType() {return BaseAnimation::AnimationType::CYCLE;}

private:
	keytime_t interval_; 

};

#endif