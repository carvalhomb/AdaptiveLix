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


Exposer::Exposer(GameData _data, Poco::NotificationQueue* _nq)
{
	data = _data;
	nq = _nq;
	sessionid = gloB->exposer_sessionid;
}


void Exposer::run()
{
	GameEventNotification* notification_msg = new GameEventNotification;
	notification_msg->load_data(data);
	notification_msg->load_sessionid(sessionid);
	nq->enqueueNotification(notification_msg);
}


