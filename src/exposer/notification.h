#pragma once

#include <Poco/Notification.h>
//#include <Poco/NotificationCenter.h>
//#include <Poco/AutoPtr.h>

#include "gamedata.h"


class GameEventNotification : public Poco::Notification
{
public:
	void load(GameData passed_data);
	GameData data;
};


