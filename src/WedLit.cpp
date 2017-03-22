#include "NodeController.h"
#include "AnimationController.h"
#include "AnimationFactory.h"
#include "WedLit.h"

#include <vector>
#include <iostream>
#include <unistd.h>

//static auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
//std::shared_ptr<spdlog::sinks::ansicolor_sink> color_sink = std::make_shared<spdlog::sinks::ansicolor_sink>(stdout_sink);

INIT_LOG(MAIN);

int main(int argc, char** argv)
{

	std::cout << "WedLit Controller" << std::endl;
	std::cout << "Version: " << WedLit_VERSION_MAJOR << "." << WedLit_VERSION_MINOR << std::endl;
	std::cout << "By Zhaf - 2017" << std::endl;

	spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l)
	{
		l->set_level(spdlog::level::trace);
	});
	
	std::shared_ptr<NodeController> nc = std::make_shared<NodeController>("192.168.2.255", 1234);
	nc->init();

	if (argc > 1)
	{
		std::string cmd(argv[1]);
		if (cmd == "stop")
		{
			nc->stop();
			nc->init();
			nc->off();
			exit(0);
		}
	}

	AnimationController ac(std::string("data/animations/BrownEyedGirl.json"), nc);

	ac.start();
	

}