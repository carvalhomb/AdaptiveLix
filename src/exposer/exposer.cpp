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

//#include <Poco/ThreadPool.h>
#include <Poco/NotificationCenter.h>

#include "locsaver.h"
#include "netsaver.h"

#include "exposer.h"

#include "notification.h"

//Exposer::Exposer() {}

//Exposer::Exposer(GameData passed_data)
//{
//	//Poco::NotificationCenter nc;
//	data = passed_data;
//	nc = &NotifCenter::static_nc;
//}

Exposer::Exposer(GameData passed_data, Poco::NotificationCenter* passed_nc)
{
	data = passed_data;
	nc = passed_nc;
}


void Exposer::run()
{
	//NetworkSaver networksaver = NetworkSaver(data);
	//LocalSaver localsaver = LocalSaver(data);

	GameEventNotification* notification_msg = new GameEventNotification;
	notification_msg->load(data);
	nc->postNotification(notification_msg);

}


