#ifndef I_PLAYER_H
#define I_PLAYER_H

class IPlayer
{
public:
	virtual ~IPlayer(){}

	virtual bool play() = 0;
	virtual bool stop() = 0;

};

#endif