#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <cstdint>
#include <limits>

using keytime_t = uint32_t;
using AnimationChannel = uint8_t;
static constexpr keytime_t KEYTIME_MAX = std::numeric_limits<keytime_t>::max();

struct Keyframe
{
	keytime_t time;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t padding; //do not remove yet.
};

struct Keyframe_pck 
{
	Keyframe_pck() : time(0),r(0),g(0),b(0){}

	Keyframe_pck(const Keyframe& k) :
	time(k.time),r(k.r),g(k.g),b(k.b)
	{

	}
	
	Keyframe_pck& operator=(const Keyframe k)
	{
		time = k.time;
		r = k.r;
		g = k.g;
		b = k.b;
		return *this;
	}

	keytime_t time;
	uint8_t r;
	uint8_t g;
	uint8_t b;
} __attribute__ ((packed));

struct KeyIndex
{
	uint8_t nKeys;
	uint8_t chId;
	uint16_t keys_offset;
};

#endif

