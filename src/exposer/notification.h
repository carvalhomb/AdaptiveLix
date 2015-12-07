#pragma once

#include <Poco/Notification.h>

#include "gamedata.h"


class GameEventNotification : public Poco::Notification
{
public:
	void load_data(GameData data);
	void load_sessionid(std::string sessionid);
	std::string sessionid;
	GameData data;
private:

};


// quit notification send to worker thread
class QuitNotification: public Poco::Notification
{
public:
	typedef Poco::AutoPtr<QuitNotification> Ptr;
};
