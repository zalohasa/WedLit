#ifndef ANIMATION_CONTROLLER_H
#define ANIMATION_CONTROLLER_H

#include <QByteArray>
#include <QJsonObject>

#include <string>
#include <memory>
#include <thread>

class NodeController;
class IPlayer;
class BaseAnimation;

class AnimationController
{
	
public:

	AnimationController();
	virtual ~AnimationController();
	explicit AnimationController(std::string jsonFilename, std::shared_ptr<NodeController> nodeCtrl);
	explicit AnimationController(QByteArray jsonData);

	void setAnimation(std::unique_ptr<BaseAnimation> animation);
	void setNodeController(std::shared_ptr<NodeController> nController);

	void start();
	void stop();
	bool isValid();
	bool isPlaying();


private:
	static void play_thread_entry(AnimationController* ac);
	void internal_start();
	void initializeFromJson(QByteArray data);
	void parseAnimation(QJsonObject anim);
	void parseChannelMap(QJsonObject map);
	void initializeAudio(QString filename);
	std::unique_ptr<BaseAnimation> animation_;
	std::shared_ptr<NodeController> nodeController_;
	std::unique_ptr<IPlayer> player_;
	std::string audioFile_;
	bool valid_;
	bool playing_;
	std::thread playThread_;

};

#endif //ANIMATION_CONTROLLER_H