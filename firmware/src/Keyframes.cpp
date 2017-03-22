#include "Keyframes.h"

Keyframe* Keyframes::first = nullptr;
Keyframe* Keyframes::last = nullptr;
size_t Keyframes::internalSize = 0;

void Keyframes::insertKeyframe(uint32_t time, uint8_t r, uint8_t g, uint8_t b)
{
  if (last)
  {
    //Check the new keyframe is higher than the last we already have
    if (time < last->time)
    {
      Serial.println("Filter out an old keyframe\n");
      return;
    }
  }
  Keyframe* newk = new Keyframe();
  newk->time = time;
  newk->r = r;
  newk->g = g;
  newk->b = b;
  newk->next = nullptr;
  if (first == nullptr)
  {
    //Initialize the list with the first keyframe.
    first = newk;
    last = newk;
  }
  else
  {
    last->next = newk;
    last = newk;
  }
  internalSize++;
}

Keyframe* Keyframes::getFirst()
{
  return first;
}

Keyframe* Keyframes::getLast()
{
  return last;
}

size_t Keyframes::size()
{
  return internalSize;
}

void Keyframes::clearAll()
{
  size_t originalSize = internalSize;
  for (int i = 0; i < originalSize; i++)
  {
    removeFirst();
  }
}

void Keyframes::removeFirst()
{
  if (first != nullptr)
  {
    Keyframe* nextToFirst = first->next;
    delete first;
    first = nextToFirst;
    internalSize--;
    if (first == nullptr)
    {
      last = nullptr;
    }
  }
}

void Keyframes::print()
{
  Keyframe* start = first;
  while (start!=nullptr)
  {
    Serial.println("-------");
    Serial.print("  Time: ");
    Serial.println(start->time);
    Serial.print("  R: ");
    Serial.println(start->r);
    Serial.print("  G: ");
    Serial.println(start->g);
    Serial.print("  B: ");
    Serial.println(start->b);
    start = start->next;
    optimistic_yield(1000); //Avoid WDT soft reset
  }
  Serial.println("-------");
  Serial.print("Number of keyframes: ");
  Serial.println(internalSize);
}
