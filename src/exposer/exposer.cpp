/*
 * exposer.cpp
 *
 *  Created on: 24 Nov 2015
 *      Author: maira
 */

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning


#include <string>
#include <sstream>
#include <exception>

#include "../other/globals.h"
#include "../other/file/log.h"

#include "locsaver.h"
#include "netsaver.h"

#include "exposer.h"

#include "notification.h"


Exposer::Exposer(GameData data, Poco::NotificationQueue* nq)
{
	_data = data;
	_nq = nq;
}


void Exposer::run()
{
	GameEventNotification* notification_msg = new GameEventNotification;
	notification_msg->load(_data);
	_nq->enqueueNotification(notification_msg);
}


