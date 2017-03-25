#ifndef COMMANDS_H
#define COMMANDS_H

#include "WedLit.h"

namespace Commands
{
	std::vector<std::string> getAnimationsList();	
	std::vector<std::string> getNodeList();
	void playAnimation(std::string animationName);
	void stop();
	void off();
}

#endif
