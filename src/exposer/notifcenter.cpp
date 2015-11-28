/*
 * notifcenter.cpp
 *
 *  Created on: 28 Nov 2015
 *      Author: maira
 */
#include <Poco/NotificationCenter.h>
#include <Poco/Notification.h>
#include <Poco/Observer.h>
#include <Poco/NObserver.h>
#include <Poco/AutoPtr.h>
#include <Poco/ThreadPool.h>

#include "locsaver.h"
#include "netsaver.h"
#include "gamedata.h"

#include "notifcenter.h"
#include "../other/file/log.h"

Poco::NotificationCenter NotifCenter::static_nc;

void BaseNotification::load(GameData passed_data) {
	data = passed_data;
}

void Target::handleBase(BaseNotification* pNf)
	{
	Log::log(Log::INFO, "Received notification, yay!!!");
		//std::cout << "handleBase: " << pNf->name() << std::endl;
		NetworkSaver networksaver = NetworkSaver(pNf->data);
		LocalSaver localsaver = LocalSaver(pNf->data);
		Poco::ThreadPool::defaultPool().start(networksaver);
		Poco::ThreadPool::defaultPool().start(localsaver);
		Poco::ThreadPool::defaultPool().joinAll();
		Log::log(Log::INFO, "Processed everything, are we done?");
		pNf->release(); // we got ownership, so we must release
	};

//void Target::handleSub(const Poco::AutoPtr<SubNotification>& pNf)
//	{
//		std::cout << "handleSub: " << pNf->name() << std::endl;
//	};


