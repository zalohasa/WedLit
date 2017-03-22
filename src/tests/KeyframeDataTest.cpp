#include "TestAnimation.h"
#include "KeyframeData.h"
#include "BasicTypes.h"

#include <gtest/gtest.h>

class KeyframeDataTest: public ::testing::Test
{
protected:

	KeyframeDataTest(){test.reset();}

	TestAnimation test;
};

uint8_t rightData[] = {0x01, 0x00, 0x10, 0x00, 0x01, 
	0x01, 0x18, 0x00, 0x01, 0x02, 0x20, 0x00, 0x01,
    0x03, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00};

uint8_t rightData2[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

TEST_F(KeyframeDataTest, RightInitialization_multi)
{
	int nKeysPerCh = 1;
	BaseAnimation::ChannelKeyList list;
	test.getNKeys(list, nKeysPerCh);
	KeyframeData kData(list);
	ASSERT_EQ(kData.getNumberOfChannels(), 4);
	ASSERT_EQ(kData.getSize(), sizeof(Keyframe)*(4*1) + sizeof(KeyIndex)*4);
	ASSERT_EQ(kData.getDataType(), KeyframeData::DataType::DATA_MULTI);

	const uint8_t* data = kData.getRawData();
	for (int i = 0; i < kData.getSize(); ++i)
	{
		ASSERT_EQ(data[i], rightData[i]);	
	}
}

TEST_F(KeyframeDataTest, RightInitialization_single)
{
	int nKeysPerCh = 20; //The animation test has 5 keys in the first channel
	BaseAnimation::ChannelKeyList list;
	test.getNKeys(list, nKeysPerCh);
	KeyframeData kData(list.front());
	ASSERT_EQ(kData.getNumberOfChannels(), 1);
	ASSERT_EQ(kData.getSize(), 1 + (sizeof(Keyframe) * 5));
	ASSERT_EQ(kData.getDataType(), KeyframeData::DataType::DATA_SINGLE);

	const uint8_t* data = kData.getRawData();
	ASSERT_EQ(data[0], list.front()->size());

	for (int i = 1; i < kData.getSize(); ++i)
	{
		ASSERT_EQ(data[i], rightData2[i-1]);
	}
	printf("\n");
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}