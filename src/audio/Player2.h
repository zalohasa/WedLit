#ifndef PLAYER_2_H
#define PLAYER_2_H

#include "IPlayer.h"

#include <ao/ao.h>
#include <mpg123.h>
#include <string>
#include <thread>
#include <condition_variable>

class Player2 : public IPlayer
{
public:

	Player2(const std::string& filename);
	~Player2();

	bool play();
	bool stop();
	
private:

	static void play_internal(Player2* player);
	std::string filename_;
	bool stop_;
	std::thread bck_;
	ao_device *dev_;
	mpg123_handle* mh_;

	bool playing_;
	std::mutex lock_;
	std::condition_variable cond_;

};

#endif