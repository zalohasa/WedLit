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

