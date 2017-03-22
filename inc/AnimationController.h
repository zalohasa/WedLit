#ifndef ANIMATION_CONTROLLER_H
#define ANIMATION_CONTROLLER_H

#include <string>
#include <memory>
#include <QByteArray>
#include <QMediaPlayer>
#include <QJsonObject>

class NodeController;
class IPlayer;
class BaseAnimation;

class AnimationController : public QObject
{
	
public:

	AnimationController();
	virtual ~AnimationController();
	explicit AnimationController(std::string jsonFilename, std::shared_ptr<NodeController> nodeCtrl);
	explicit AnimationController(QByteArray jsonData);

	void setAnimation(std::unique_ptr<BaseAnimation> animation);
	void setNodeController(std::shared_ptr<NodeController> nController);

public slots:
	void start();
	void stop();
	bool isValid();
	bool isPlaying();


private:
	void initializeFromJson(QByteArray data);
	void parseAnimation(QJsonObject anim);
	void parseChannelMap(QJsonObject map);
	void initializeAudio(QString filename);
	std::unique_ptr<BaseAnimation> animation_;
	std::shared_ptr<NodeController> nodeController_;
	IPlayer* player_;
	std::string audioFile_;
	bool valid_;
	bool playing_;

};

#endif //ANIMATION_CONTROLLER_H