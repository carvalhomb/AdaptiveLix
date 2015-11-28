/*
 * notifhandler.cpp
 *
 *  Created on: 28 Nov 2015
 *      Author: maira
 */
//#include <Poco/NotificationCenter.h>
//#include <Poco/Notification.h>
//#include <Poco/Observer.h>
//#include <Poco/NObserver.h>
//#include <Poco/AutoPtr.h>
#include <Poco/ThreadPool.h>

#include "../other/file/log.h"
#include "notification.h"
#include "locsaver.h"
#include "netsaver.h"

#include "notifhandler.h"



void NotificationHandler::handle(GameEventNotification* pNf)
{
	//Log::log(Log::INFO, "Received notification, yay!!!");
	NetworkSaver networksaver = NetworkSaver(pNf->data);
	LocalSaver localsaver = LocalSaver(pNf->data);
//	localsaver.run();
//	networksaver.run();
	Poco::ThreadPool::defaultPool().start(networksaver);
	Poco::ThreadPool::defaultPool().start(localsaver);
	Poco::ThreadPool::defaultPool().joinAll();
	//Log::log(Log::INFO, "Processed everything, are we done?");
	pNf->release(); // we got ownership, so we must release
};


