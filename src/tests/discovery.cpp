#include "NodeController.h"
#include "BaseAnimation.h"
#include "spdlog/spdlog.h"

#include <vector>
#include <iostream>
#include <unistd.h>



int main(void)
{
	console->set_level(spdlog::level::debug);
	console->info("Arrancando ando");
	NodeController nc("192.168.2.255", 1234);
	nc.init();

	BaseAnimation::ChannelKeyList list;
	Keyframe a = {0, 0, 0, 0, 0};
	Keyframe b = {500, 0x10, 0, 0, 0};
	Keyframe c = {1000, 0, 0x10, 0, 0};
	Keyframe d = {1500, 0, 0, 0x10, 0};
	Keyframe e = {2000, 0, 0, 0, 0};

	BaseAnimation::SharedKeyList keys(new BaseAnimation::KeyList());
	keys->push_back(a);
	keys->push_back(b);
	keys->push_back(c);
	keys->push_back(d);
	keys->push_back(e);

	list.push_back(keys);

	for (auto node : nc.getNodes())
	{
		std::cout << node << std::endl;
	}

	nc.clear();

	//nc.setChannel("17b8aa", 0);
	nc.sendInitialKeys(list);
	nc.cycle();

	sleep(10);
	nc.stop();
	nc.off();	
}