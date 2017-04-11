#include "NodeController.h"
#include "AnimationController.h"
#include "AnimationFactory.h"
#include "TcpSocket.h"
#include "WedLit.h"
#include "Player2.h"
#include "Server.h"

#include <vector>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <QDir>

//static auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
//std::shared_ptr<spdlog::sinks::ansicolor_sink> color_sink = std::make_shared<spdlog::sinks::ansicolor_sink>(stdout_sink);

INIT_LOG(MAIN);

WedLitContext appContext;

extern "C"
{
	void signal_handler(int signal)
	{
		appContext.exitReq = true;
	}
}

int main(int argc, char** argv)
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = signal_handler;

	sigaction(SIGTERM, &act, 0);
	sigaction(SIGINT, &act, 0);

	std::cout << "WedLit Controller" << std::endl;
	std::cout << "Version: " << WedLit_VERSION_MAJOR << "." << WedLit_VERSION_MINOR << std::endl;
	std::cout << "By Zhaf - 2017" << std::endl;

	spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l)
	{
		l->set_level(spdlog::level::trace);
	});

	DEBUG("Current dir: {}", QDir::currentPath().toStdString());

	//initialize audio
	//TODO make this player agnostic.
	Player2::init();

	Server srv(3210);

	appContext.node_controller = std::make_shared<NodeController>("10.0.4.255", 1234);
	appContext.node_controller->init();
	appContext.node_controller->startDiscoveryThread();

	srv.exec();

	DEBUG("Cleaning up...");

	Player2::shutdown();

}