#ifndef ANIMATION_FACTORY_H
#define ANIMATION_FACTORY_H

#include "BaseAnimation.h"

class AnimationFactory
{
public:
	//TODO now the AnimationFactory has all its method static, because there is no 
	//dynamic registration capability yet. Once its implemented, the Factory should become 
	//a singleton, or instanciated when needed (unless a static registratio method is implemented)

	static std::unique_ptr<BaseAnimation> createAnimation(const std::string& name, const BaseAnimation::AnimationParameters& params);
};

#endif