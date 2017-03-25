#include "Commands.h"
#include "AnimationController.h"
#include "NodeController.h"

#include <QDir>
#include <QString>

INIT_LOG(COMMANDS);

namespace Commands
{
	std::vector<std::string> getAnimationsList()
	{
		std::vector<std::string> res;
		QDir dir((QString(FOLDER_ANIMATIONS)));
		if (!dir.exists())
		{
			return res;
		}

		QStringList filters;
		filters << "*.json" << "*.JSON";
		dir.setNameFilters(filters);

		QStringList names = dir.entryList();

		for (int i = 0; i < names.size(); ++i)
		{
			res.push_back(names.at(i).toStdString());
		}

		for (auto s : res)
		{
			TRACE("File found: {}", s);
		}

		return res;
	}

	std::vector<std::string> getNodeList()
	{
		std::vector<std::string> res;
		if (appContext.node_controller)
		{
			std::vector<NodeController::NodeId> nodes = appContext.node_controller->getNodes();
			for (auto s : nodes)
			{
				res.push_back(s);
			}
		}
		return res;
	}

	void playAnimation(std::string animationName)
	{
		WedLitContext& a = appContext;
		if (a.anim_controller && a.anim_controller->isPlaying())
		{
			WARN("Can't play an animation, there is one already playing");
			return;
		}

		DEBUG("Creating new animation for file: {}", animationName);
		a.anim_controller = std::make_shared<AnimationController>(animationName, a.node_controller);
		a.anim_controller->start();
	}

	void stop()
	{
		if (appContext.anim_controller && appContext.anim_controller->isPlaying())
		{
			INFO("Stoping animation...");
			appContext.anim_controller->stop();
		}
		else
		{
			WARN("Can't stop an animation because there is none playing");
		}
	}

	void off()
	{
		if (appContext.node_controller)
		{
			appContext.node_controller->off();
		}
	}
}