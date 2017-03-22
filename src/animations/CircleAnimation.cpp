#include "CircleAnimation.h"

#include <string>

CircleAnimation::CircleAnimation(const BaseAnimation::AnimationParameters& params)
{
	if (params.find("interval") != params.end())
	{
		interval_ = std::stoul(params.at("interval"));
	}
	else
	{
		interval_ = 300;
	}

	if (params.find("channels") != params.end())
	{
		nChannels_ = std::stoul(params.at("channels"));
	}
	else
	{
		nChannels_ = 3;
	}

	if (params.find("level") != params.end())
	{
		level_ = std::stoul(params.at("level"));
	}
	else
	{
		level_ = 0x10;
	}

	for (int i = 0; i < nChannels_; ++i)
	{
		BaseAnimation::SharedKeyList keyList = std::make_shared<KeyList>(nChannels_ + 1);
		for (int j = 0; j < nChannels_+1; j++)
		{
			if (j == i)
			{
				(*keyList)[j].time = interval_ * j;
				(*keyList)[j].r = level_;
				(*keyList)[j].g = level_;
				(*keyList)[j].b = level_;
			}
			else
			{
				(*keyList)[j].time = interval_ * j;
				(*keyList)[j].r = 0;
				(*keyList)[j].g = 0;
				(*keyList)[j].b = 0;
			}
		}
		data_.push_back(keyList);
	}

	//Last keyframe of first channel should be high again (to cycle)
	data_[0]->at(nChannels_).r = level_;
	data_[0]->at(nChannels_).g = level_;
	data_[0]->at(nChannels_).b = level_;

	init();

}