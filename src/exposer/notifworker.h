/*
 * notifworker.h
 *
 *  Created on: 28 Nov 2015
 *      Author: maira
 */

#pragma once

#include <Poco/NotificationQueue.h>
#include <Poco/RWLock.h>
#include <Poco/Runnable.h>

class NotificationWorker: public Poco::Runnable
{
public:
	NotificationWorker(Poco::NotificationQueue* queue, Poco::RWLock* lock);
	void run();
private:
	Poco::NotificationQueue* _queue;
	Poco::RWLock* _lock;
};
