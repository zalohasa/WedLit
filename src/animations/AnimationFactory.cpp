#include "AnimationFactory.h"
#include "NullAnimation.h"
#include "ThreeColorBasicAnimation.h"
#include "CircleAnimation.h"
#include "FileAnimation.h"
#include "WedLit.h"

#include <iostream>

INIT_LOG(ANIM_FACTORY);

std::unique_ptr<BaseAnimation> AnimationFactory::createAnimation(const std::string &name, const BaseAnimation::AnimationParameters &params)
{
	DEBUG("Creating animatino with type: {}", name);
	for (auto p : params)
	{
		TRACE("Parameter: {} : {}", p.first, p.second);
	}
	//Check the animation type requested:
	if (name == "nullanimation")
	{
		return std::unique_ptr<BaseAnimation>(new NullAnimation(params));
	}
	else if (name == "threecolorbasic")
	{
		return std::unique_ptr<BaseAnimation>(new ThreeColorBasicAnimation(params));
	}
	else if (name == "circle")
	{
		return std::unique_ptr<BaseAnimation>(new CircleAnimation(params));
	}
	else if (name == "file")
	{
		return std::unique_ptr<BaseAnimation>(new FileAnimation(params));
	}
	else
	{
		ERROR("Animation name not recognized.");
		
	}
	return std::unique_ptr<BaseAnimation>();
}