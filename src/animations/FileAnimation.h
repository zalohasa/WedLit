#ifndef FILE_ANIMATION_H
#define FILE_ANIMATION_H

#include "BaseAnimation.h"

class FileAnimation : public BaseAnimation
{
public:
	FileAnimation(const AnimationParameters& params);
	~FileAnimation(){}

	virtual AnimationType getAnimType() { return BaseAnimation::AnimationType::SINGLE; }

private:
	struct KeyframeFileIndex 
	{
		uint8_t nChannels;
		uint32_t nKeyframes[];
	};

};


#endif