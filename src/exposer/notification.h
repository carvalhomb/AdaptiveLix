#pragma once

#include <Poco/Notification.h>

#include "gamedata.h"


class GameEventNotification : public Poco::Notification
{
public:
	void load(GameData passed_data);
	GameData data;
};


// quit notification send to worker thread
class QuitNotification: public Poco::Notification
{
public:
	typedef Poco::AutoPtr<QuitNotification> Ptr;
};
