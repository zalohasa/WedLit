#include <Arduino.h>
class Keyframe
{
public:
  uint32_t time;
  Keyframe* next;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t padding;
};

class IndexFrame
{
public:
  uint8_t nKeys;
  uint8_t node_id;
  uint16_t keys_offset;
};

class Keyframes
{
public:
  static void insertKeyframe(uint32_t time, uint8_t r, uint8_t g, uint8_t b);
  static Keyframe* getFirst();
  static Keyframe* getLast();
  static void removeFirst();
  static size_t size();
  static void print();
  static void clearAll();
private:
  static Keyframe* first;
  static Keyframe* last;
  static size_t internalSize;
};
