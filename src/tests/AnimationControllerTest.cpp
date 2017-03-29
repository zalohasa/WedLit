#include "AnimationController.h"
#include "spdlog/spdlog.h"

#include <QByteArray>
#include <QDir>

#include <gtest/gtest.h>

static auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
std::shared_ptr<spdlog::sinks::ansicolor_sink> color_sink = std::make_shared<spdlog::sinks::ansicolor_sink>(stdout_sink);

const char* json1 = " \
{\
	\"animation\": \
	{\
		\"type\": \"nullanimation\",\
		\"param1\": \"param1\",\
		\"param2\": \"param2\"\
	}\
}\
";

const char* json2 = " \
{\
	\"animation\": \
	{\
		\"type\": \"test\",\
		\"param1\": \"param1\",\
		\"param2\": \"param2\"\
	}\
}\
";

TEST(AnimControllerBasicTest, TestQByteArray)
{
	QByteArray data(json1);
	AnimationController ac(data);
	ASSERT_TRUE(ac.isValid());
}

TEST(AnimControllerBasicTest, TestJSonFile)
{
	AnimationController ac(std::string("animctest1.json"), nullptr);
	ASSERT_TRUE(ac.isValid());
}

TEST(AnimControllerBasicTest, TestInvalidAnimation)
{
	QByteArray data(json2);
	AnimationController ac(data);
	ASSERT_FALSE(ac.isValid());
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}