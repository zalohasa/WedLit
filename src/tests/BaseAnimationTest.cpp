#include "TestAnimation.h"


#include <gtest/gtest.h>

#ifdef TEST_VERBOSE
#include <iostream>
#endif

static constexpr std::vector<Keyframe>::size_type END = std::numeric_limits<std::vector<Keyframe>::size_type>::max();

void printKeyframeData(BaseAnimation::ChannelKeyList& data)
{
#ifdef TEST_VERBOSE
	std::cout << "Keyframe times:" << std::endl;
	for (auto channelList : data)
	{
		for (size_t i = 0; i < channelList->size(); ++i)
		{
			std::cout << (*channelList)[i].time << "\t";
		}
		std::cout << std::endl;
	}
#endif
}

void printKeyframePositions(std::vector<std::vector<Keyframe>::size_type>& pos)
{
#ifdef TEST_VERBOSE
	std::cout << "Keyframe positions: " << std::endl;

	for (auto idx : pos)
	{
		if (idx != END)
		{
			std::cout << idx << "\t";
		}
		else
		{
			std::cout << "END" << "\t";
		}
	}
	std::cout << std::endl;
#endif
}

class BaseAnimationTest: public ::testing::Test
{
protected:

	BaseAnimationTest(){test.reset();}

	TestAnimation test;
};

TEST_F(BaseAnimationTest, IsEmpty)
{
	auto keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i],0);
	}
}

TEST_F(BaseAnimationTest, Take3KeysEachTime)
{
	BaseAnimation::ChannelKeyList list;
	test.getNKeys(list, 3);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	size_t time = 0;
	for (auto cl : list)
	{
		ASSERT_EQ(cl->at(0).time, time);
		ASSERT_EQ(cl->size(), 3);
		++time;
	}

	auto keypos = test.getKeyframePositions();
	ASSERT_EQ(keypos[0], keypos[1]);
	ASSERT_EQ(keypos[1], keypos[2]);
	ASSERT_EQ(keypos[2], keypos[3]);
	ASSERT_EQ(keypos[3], 2);

	ASSERT_EQ(test.getFirstKeyout(), 2);

	list.clear();
	test.getNKeys(list, 3);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	ASSERT_EQ(keypos[0], END);
	ASSERT_EQ(keypos[1], END);
	ASSERT_EQ(keypos[2], 5);
	ASSERT_EQ(keypos[3], 5);

	auto first = list.begin();
	ASSERT_EQ((*first)->size(), 2);
	++first;
	ASSERT_EQ((*first)->size(), 3);
	++first;
	ASSERT_EQ((*first)->size(), 3);
	++first;
	ASSERT_EQ((*first)->size(), 3);

	ASSERT_EQ(test.getFirstKeyout(), 7);

	list.clear();
	test.getNKeys(list, 3);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], END);
	}

	first = list.begin();
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 1);
	++first;
	ASSERT_EQ((*first)->size(), 2);

	ASSERT_EQ(test.getFirstKeyout(), KEYTIME_MAX);

}

TEST_F(BaseAnimationTest, Take1KeyEachTime)
{
	BaseAnimation::ChannelKeyList list;
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	auto keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], 0);
	}

	for (auto cl : list)
	{
		ASSERT_EQ(cl->size(), 1);
	}
	ASSERT_EQ(test.getFirstKeyout(), 0);
	
	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], 1);
	}

	for (auto cl : list)
	{
		ASSERT_EQ(cl->size(), 1);
	}
	ASSERT_EQ(test.getFirstKeyout(), 1);

	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], 2);
	}

	for (auto cl : list)
	{
		ASSERT_EQ(cl->size(), 1);
	}
	ASSERT_EQ(test.getFirstKeyout(), 2);

	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], 3);
	}

	for (auto cl : list)
	{
		ASSERT_EQ(cl->size(), 1);
	}
	ASSERT_EQ(test.getFirstKeyout(), 3);

	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	ASSERT_EQ(keypos[0], END);
	ASSERT_EQ(keypos[1], 4);
	ASSERT_EQ(keypos[2], 4);
	ASSERT_EQ(keypos[3], 4);

	for (auto cl : list)
	{
		ASSERT_EQ(cl->size(), 1);
	}

	ASSERT_EQ(test.getFirstKeyout(), 5);

	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	ASSERT_EQ(keypos[0], END);
	ASSERT_EQ(keypos[1], END);
	ASSERT_EQ(keypos[2], 5);
	ASSERT_EQ(keypos[3], 5);
	
	auto first = list.begin();
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 1);
	++first;
	ASSERT_EQ((*first)->size(), 1);
	++first;
	ASSERT_EQ((*first)->size(), 1);

	ASSERT_EQ(test.getFirstKeyout(), 7);

	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	ASSERT_EQ(keypos[0], END);
	ASSERT_EQ(keypos[1], END);
	ASSERT_EQ(keypos[2], END);
	ASSERT_EQ(keypos[3], 6);
	
	first = list.begin();
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 1);
	++first;
	ASSERT_EQ((*first)->size(), 1);

	ASSERT_EQ(test.getFirstKeyout(), 9);

	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	ASSERT_EQ(keypos[0], END);
	ASSERT_EQ(keypos[1], END);
	ASSERT_EQ(keypos[2], END);
	ASSERT_EQ(keypos[3], END);
	
	first = list.begin();
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 1);

	ASSERT_EQ(test.getFirstKeyout(), KEYTIME_MAX);

	list.clear();
	test.getNKeys(list, 1);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], END);
	}
	
	first = list.begin();
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 0);
	++first;
	ASSERT_EQ((*first)->size(), 0);

	ASSERT_EQ(test.getFirstKeyout(), KEYTIME_MAX);

}

TEST_F(BaseAnimationTest, Take20KeyEachTime)
{
	BaseAnimation::ChannelKeyList list;
	test.getNKeys(list, 20);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	auto keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], END);
	}

	auto first = list.begin();
	ASSERT_EQ((*first)->size(), 5);
	++first;
	ASSERT_EQ((*first)->size(), 6);
	++first;
	ASSERT_EQ((*first)->size(), 7);
	++first;
	ASSERT_EQ((*first)->size(), 8);

	ASSERT_EQ(test.getFirstKeyout(), KEYTIME_MAX);

}

TEST_F(BaseAnimationTest, ValidateReset)
{
	BaseAnimation::ChannelKeyList list;
	test.getNKeys(list, 20);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	auto keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], END);
	}

	auto first = list.begin();
	ASSERT_EQ((*first)->size(), 5);
	++first;
	ASSERT_EQ((*first)->size(), 6);
	++first;
	ASSERT_EQ((*first)->size(), 7);
	++first;
	ASSERT_EQ((*first)->size(), 8);

	ASSERT_EQ(test.getFirstKeyout(), KEYTIME_MAX);

	test.reset();
	keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], 0);
	}

	list.clear();
	test.getNKeys(list, 20);
	printKeyframeData(list);
	printKeyframePositions(test.getKeyframePositions());

	keypos = test.getKeyframePositions();
	for (size_t i = 0; i < keypos.size(); ++i)
	{
		ASSERT_EQ(keypos[i], END);
	}

	first = list.begin();
	ASSERT_EQ((*first)->size(), 5);
	++first;
	ASSERT_EQ((*first)->size(), 6);
	++first;
	ASSERT_EQ((*first)->size(), 7);
	++first;
	ASSERT_EQ((*first)->size(), 8);

	ASSERT_EQ(test.getFirstKeyout(), KEYTIME_MAX);
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}