#include "FileAnimation.h"
#include "WedLit.h"

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

INIT_LOG(FILE_ANIMATION);

FileAnimation::FileAnimation(const BaseAnimation::AnimationParameters& params)
{
	if (params.find("file") != params.end())
	{
		std::string filename = params.at("file");
		ifstream file (filename, ios::in|ios::binary);
		if (file.is_open())
		{
			INFO("Reading animation file {}", filename);
			uint8_t nChannels;
			file.read((char*)&nChannels, sizeof(uint8_t));
			DEBUG("Reading {} channels", nChannels);
			uint32_t *keyframes = new uint32_t[nChannels];
			file.read((char*)keyframes, sizeof(uint32_t)*nChannels);

			Keyframe_pck* readKey = new Keyframe_pck();
			for (int ch = 0; ch < nChannels; ++ch)
			{
				BaseAnimation::SharedKeyList keyList = std::make_shared<KeyList>(keyframes[ch]);
				TRACE("Keyframes in channel {} : {}", ch, keyframes[ch]);
				for (int k = 0; k < keyframes[ch]; ++k)
				{
					file.read((char*)readKey, sizeof(Keyframe_pck));
					(*keyList)[k].time = readKey->time;
					(*keyList)[k].r = readKey->r;
					(*keyList)[k].g = readKey->g;
					(*keyList)[k].b = readKey->b;
				}
				data_.push_back(keyList);
			}
			file.close();
		}
		else
		{
			ERROR("Can't open the file {}", filename);
		}
	}
	else
	{
		ERROR("No file parameter found.");
	}

	init();
}