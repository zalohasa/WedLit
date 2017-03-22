#ifndef NULL_ANIMATION_H
#define NULL_ANIMATION_H

//Empty animation with no keyframes.

#include "BaseAnimation.h"

class NullAnimation : public BaseAnimation
{
public:
	NullAnimation();
	NullAnimation(const BaseAnimation::AnimationParameters& parameters);
	~NullAnimation();

	virtual AnimationType getAnimType();
	const BaseAnimation::AnimationParameters& getParameters();
private:
	BaseAnimation::AnimationParameters params_;
};

#endif