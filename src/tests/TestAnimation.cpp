#include "TestAnimation.h"

static constexpr int NUMBER_OF_CHANNELS = 4;
static constexpr int NUMBER_OF_MINIM_KEYS_PER_CHANNEL = 5;

// Sample animation implementation with the following keyframes and keyframe times:
// Keyframe times:
// 0	1	2	3	4	
// 1	2	3	4	5	6	
// 2	3	4	5	6	7	8	
// 3	4	5	6	7	8	9	10	

TestAnimation::TestAnimation()
{
	//4 channel data.
	for (int i = 0; i < NUMBER_OF_CHANNELS; ++i)
	{
		BaseAnimation::SharedKeyList keyList = std::make_shared<KeyList>(i+NUMBER_OF_MINIM_KEYS_PER_CHANNEL);
		for (int j = 0; j < (i + NUMBER_OF_MINIM_KEYS_PER_CHANNEL); ++j)
		{
			(*keyList)[j].time = j + i;//Set the keyframe time to the keyframe index + keyframe channel
		}
		data_.push_back(keyList); 
	}
	init();
	
}