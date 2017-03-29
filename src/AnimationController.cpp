#include "WedLit.h"
#include "AnimationController.h"
#include "NodeController.h"
#include "AnimationFactory.h"
#include "BaseAnimation.h"
#include "Player2.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QStringList>
#include <QFile>
#include <QDir>

#include <iostream>
#include <cassert>
#include <unordered_map>
#include <ctime>
#include <cmath>
#include <unistd.h>
#include <chrono>

INIT_LOG(ANIM_CONTR);

void AnimationController::play_thread_entry(AnimationController* ctrl)
{
	ctrl->internal_start();
}

AnimationController::AnimationController() : 
	player_(nullptr),
	valid_(true),
	playing_(false)
{

}

AnimationController::~AnimationController()
{
	if (playThread_.joinable())
	{
		DEBUG("Waiting for animation controller thread to finish");
		playThread_.join();
	}
}

AnimationController::AnimationController(std::string jsonFilename, std::shared_ptr<NodeController> nodeCtrl) : 
	nodeController_(nodeCtrl),
	player_(nullptr),
	valid_(true),
	playing_(false)
{
	jsonFilename = FOLDER_ANIMATIONS + jsonFilename;
	QDir dir(jsonFilename.c_str());

	dir.makeAbsolute();
	DEBUG("File path: {}", dir.absolutePath().toStdString());

	QFile loadFile(dir.absolutePath());

    if (!loadFile.open(QIODevice::ReadOnly)) {
        ERROR("File not found: {}",jsonFilename);
        valid_ = false;
    }

    INFO("Parsing json file {}", jsonFilename);
    QByteArray saveData = loadFile.readAll();
    initializeFromJson(saveData);
}

AnimationController::AnimationController(QByteArray jsonData) : 
	player_(nullptr),
	valid_(true),
	playing_(false)
{
	initializeFromJson(jsonData);
}

void AnimationController::setAnimation(std::unique_ptr<BaseAnimation> animation)
{
	animation_ = std::move(animation);
}

void AnimationController::setNodeController(std::shared_ptr<NodeController> nController)
{
	nodeController_ = nController;
}

void AnimationController::internal_start()
{
	nodeController_->clear();

	BaseAnimation::ChannelKeyList list;
	animation_->getNKeys(list, N_KEYS_ON_INIT);

	nodeController_->sendInitialKeys(list);

	if (animation_->getAnimType() == BaseAnimation::AnimationType::SINGLE)
	{
		size_t maxKeysPerChannel = std::floor(MAX_KEYS_ON_UPDATE / animation_->getChannels());
		DEBUG("Playing a SINGLE type animation with {} keys per channel on update.", maxKeysPerChannel);

		playing_ = true;
		keytime_t keyout, lastKeyout;
		lastKeyout = 0;
						
		if (player_)
		{
			player_->play();
		}

		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		nodeController_->play();
		
		//needed wait before try to sync. 
		//TODO Revisit when a new sync gets implemented.
		usleep(200*1000); 
		nodeController_->correctDrift(startTime);

		bool firstTime = true;
		while (playing_ && (keyout = animation_->getFirstKeyout()) != KEYTIME_MAX)
		{
			list.clear();
			animation_->getNKeys(list, maxKeysPerChannel);
			keytime_t msToSleep = 0;
			if (!firstTime)
			{
				msToSleep = (keyout - lastKeyout);
			}
			else
			{
				msToSleep = (keyout - lastKeyout) / 2;
				firstTime = false;
			}
			lastKeyout = keyout;
			TRACE("Sleeping for {}ms", msToSleep);
			usleep(msToSleep*1000);
			TRACE("End of sleep, sending data");
			nodeController_->sendKeys(list);
		}
		playing_ = false;
		INFO("Animation finished");
	}
	else
	{
		DEBUG("Playing a CYCLE type animation");
		nodeController_->cycle(); 
		playing_ = true;
		return;
	}
}

void AnimationController::start()
{
	if (!animation_)
	{
		WARN("No animation registered");
		if (player_)
		{
			INFO("Playing audio only animation");
			player_->play();
			playing_ = true;
		}
		return;
	}

	if (!nodeController_)
	{
		ERROR("No node controller available");
		return;
	}

	INFO("Starting a new thread to play the animation");
	playThread_ = std::thread(&AnimationController::play_thread_entry, this);
}

void AnimationController::stop()
{
	if (player_)
	{
		player_->stop();
		player_.reset();
	}
	if (nodeController_)
	{
		nodeController_->stop();
		playing_ = false;
	}
}

bool AnimationController::isValid()
{
	return valid_;
}

bool AnimationController::isPlaying()
{
	return playing_;
}

void AnimationController::initializeFromJson(QByteArray data)
{
	QJsonParseError pError;
	QJsonDocument doc = QJsonDocument::fromJson(data, &pError);
	if (pError.error != QJsonParseError::NoError)
	{
		ERROR("Error parsing json: {} - {}", pError.errorString().toStdString(), pError.offset);
		valid_ = false;
		return;
	}
	QJsonObject mainObj = doc.object();
	
	if (mainObj.contains(QStringLiteral("animation")))
	{
		parseAnimation(mainObj["animation"].toObject());
	}
	if (mainObj.contains(QStringLiteral("channelMap")))
	{
		parseChannelMap(mainObj["channelMap"].toObject());
	}
	if (mainObj.contains(QStringLiteral("audioFile")))
	{
		initializeAudio(mainObj["audioFile"].toString());
	}

}

void AnimationController::parseAnimation(QJsonObject animation)
{
	if (animation["type"].isUndefined())
	{
		ERROR("Animation type not specified, json data not valid");
		valid_ = false;
		return;
	}

	QString qtype = animation["type"].toString();
	DEBUG("Json animation type: {}", qtype.toStdString());
	QStringList params = animation.keys();
	params.removeAll(QStringLiteral("type"));
	BaseAnimation::AnimationParameters paramMap;
	for (QString s: params)
	{
		if (animation[s].isString())
		{
			paramMap.insert(std::make_pair(s.toStdString(), animation[s].toString().toStdString()));
			TRACE("Add parameter: {} : {}",s.toStdString(), animation[s].toString().toStdString());
		}
		else
		{
			WARN("Only string parameters are allowed. Parameter: {}", s.toStdString());
		}
	}
	animation_ = std::move(AnimationFactory::createAnimation(qtype.toStdString(), paramMap));
	if (!animation_)
	{
		valid_ = false;
	}
}

void AnimationController::parseChannelMap(QJsonObject map)
{
	if (!animation_)
	{
		WARN("Can't specify channel map without animation");
		return;
	}
	if (map.contains(QStringLiteral("auto")))
	{
		TRACE("Auto parameter found");
		if (map["auto"].toBool())
		{	
			TRACE("Auto parameter true");
			//Channel selection auto. 
			if (nodeController_)
			{
				DEBUG("Setting all nodes to auto channel");
				nodeController_->setChannelAuto(animation_->getChannels());
			}
		}
	}
	else if (map.contains(QStringLiteral("all")))
	{
		TRACE("all parameter found");
		if (map["all"].isDouble())
		{
			TRACE("All parameter numeric");
			if (nodeController_)
			{
				DEBUG("Setting all nodes to channel {}", map["all"].toInt());
				nodeController_->setAllToChannel(map["all"].toInt());
			}
		}
	}
	else
	{
		QStringList params = map.keys();
		NodeController::ChannelMap chMap;
		for (QString s : params)
		{
			if (map[s].isDouble())
			{
				chMap.insert(std::make_pair(s.toStdString(), map[s].toInt()));
			}
			else
			{
				ERROR("Numeric channel needed for node {}", s.toStdString());
			}
		}
		nodeController_->setChannelMap(chMap);
	}
}

void AnimationController::initializeAudio(QString filename)
{
	audioFile_ = filename.toStdString();
	DEBUG("Initialize audio with file: {}", filename.toStdString());

	player_ = std::unique_ptr<IPlayer>(new Player2(audioFile_));

}

