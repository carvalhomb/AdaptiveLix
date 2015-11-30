/*
 * notifworker.h
 *
 *  Created on: 28 Nov 2015
 *      Author: maira
 */

#pragma once

#include <Poco/NotificationQueue.h>
#include <Poco/Runnable.h>

class NotificationWorker: public Poco::Runnable
{
public:
	NotificationWorker(Poco::NotificationQueue& queue);
	void run();
private:
	Poco::NotificationQueue& _queue;
};
