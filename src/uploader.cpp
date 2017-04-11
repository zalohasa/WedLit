#include "NodeController.h"
#include "WedLit.h"

#include <iostream>
#include <stdlib.h>

INIT_LOG(UPLOADER);

WedLitContext appContext;

int main(int argc, char** argv)
{
	std::cout << "WedLit firmware uploader" << std::endl;
	std::cout << "Version: " << WedLit_VERSION_MAJOR << "." << WedLit_VERSION_MINOR << std::endl;
	std::cout << "By Zhaf - 2017" << std::endl;

	std::shared_ptr<NodeController> ctrl = std::make_shared<NodeController>("10.0.4.255", 1234);
	ctrl->init();

	std::vector<std::string> ips = ctrl->getNodesAddr();
	INFO("Uploading firmware to {} nodes", ips.size());

	if (argc > 1)
	{
		std::string ip = std::string(argv[1]);
		INFO("Updating node with IP {}", ip);

		std::string command = "./espota.py --debug --progress -i " 
				+ ip + " -f ./firmware.bin";

		int res = system(command.c_str());
		if (res != 0)
		{
			ERROR("Error uploading to {}, retry one time.", ip);
			res = system(command.c_str());
			if (res != 0)
			{
				ERROR("Error again uploading to {}. Skiping this node", ip);
			}
		}
	}
	else
	{

		for (auto s : ips)
		{
			INFO("Uploading to node {}", s);
			std::string command = "./espota.py --debug --progress -i " 
				+ s + " -f ./firmware.bin";

			int res = system(command.c_str());
			if (res != 0)
			{
				ERROR("Error uploading to {}, retry one time.", s);
				res = system(command.c_str());
				if (res != 0)
				{
					ERROR("Error again uploading to {}. Skiping this node", s);
				}
			}
		}
	}
}