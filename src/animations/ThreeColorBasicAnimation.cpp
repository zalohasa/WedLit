#include "ThreeColorBasicAnimation.h"

#include <string>

ThreeColorBasicAnimation::ThreeColorBasicAnimation(const BaseAnimation::AnimationParameters& params) : BaseAnimation()
{
	if (params.find("interval") != params.end())
	{
		interval_ = std::stoul(params.at("interval"));
	}
	else
	{
		interval_ = 500;
	}
	BaseAnimation::SharedKeyList keyList = std::make_shared<KeyList>(5);

	(*keyList)[0].time = 0;
	(*keyList)[0].r = 0;
	(*keyList)[0].g = 0;
	(*keyList)[0].b = 0;

	(*keyList)[1].time = interval_ * 1;
	(*keyList)[1].r = 0x10;
	(*keyList)[1].g = 0;
	(*keyList)[1].b = 0;

	(*keyList)[2].time = interval_ * 2;
	(*keyList)[2].r = 0;
	(*keyList)[2].g = 0x10;
	(*keyList)[2].b = 0;	
	
	(*keyList)[3].time = interval_ * 3;
	(*keyList)[3].r = 0;
	(*keyList)[3].g = 0;
	(*keyList)[3].b = 0x10;

	(*keyList)[4].time = interval_ * 4;
	(*keyList)[4].r = 0;
	(*keyList)[4].g = 0;
	(*keyList)[4].b = 0;

	data_.push_back(keyList);
	init();

}