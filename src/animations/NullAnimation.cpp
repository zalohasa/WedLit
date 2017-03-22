#include "NullAnimation.h"

NullAnimation::NullAnimation()
{
	init();
}

NullAnimation::NullAnimation(const BaseAnimation::AnimationParameters& parameters)
{
	init();
	params_ = parameters;
}

BaseAnimation::AnimationType NullAnimation::getAnimType()
{
	return BaseAnimation::AnimationType::CYCLE;
}

NullAnimation::~NullAnimation()
{
	//Nothing to do
}

const BaseAnimation::AnimationParameters& NullAnimation::getParameters()
{
	return params_;
}