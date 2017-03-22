#ifndef PLAYER_H
#define PLAYER_H

#include "IPlayer.h"

#include <ao/ao.h>
#include <mpg123.h>
#include <string>
#include <thread>

class Player : public IPlayer
{
public:

	Player(const std::string& filename);
	~Player();

	bool play();
	bool stop();
	
private:

	static void play_internal(Player* player);
	std::string filename_;
	bool stop_;
	std::thread bck_;
	ao_device *dev_;

};

#endif