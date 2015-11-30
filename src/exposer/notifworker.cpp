/*
 * notifworker.cpp
 *
 *  Created on: 28 Nov 2015
 *      Author: maira
 */
#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <Poco/NotificationQueue.h>
#include <Poco/Notification.h>
#include <Poco/AutoPtr.h>
#include <Poco/Thread.h>


#include "notifworker.h"
#include "notification.h"
#include "locsaver.h"
#include "netsaver.h"
#include "../other/file/log.h"

NotificationWorker::NotificationWorker(Poco::NotificationQueue& queue): _queue(queue) {}

void NotificationWorker::run()
{
//	Poco::AutoPtr<Poco::Notification> pNf(_queue.waitDequeueNotification());
//
//	while (pNf)
//	{
//		GameEventNotification* pWorkNf =
//				dynamic_cast<GameEventNotification*>(pNf.get());
//		if (pWorkNf)
//		{
//			Log::log(Log::INFO, "Received notification, yay!!!");
//			NetworkSaver networksaver = NetworkSaver(pWorkNf->data);
//			LocalSaver localsaver = LocalSaver(pWorkNf->data);
//			localsaver.run();
//			networksaver.run();
//			Log::log(Log::INFO, "Processed everything, are we done?");
//		}
//		pNf = _queue.waitDequeueNotification();
//	}
	for (;;)
	{
		Poco::AutoPtr<Poco::Notification> pNf(_queue.waitDequeueNotification());
		if (pNf)
		{
			GameEventNotification* pWorkNf = dynamic_cast<GameEventNotification*>(pNf.get());
			if (pWorkNf)
			{
				{
					//Do the work
					NetworkSaver networksaver = NetworkSaver(pWorkNf->data);
					LocalSaver localsaver = LocalSaver(pWorkNf->data);
					localsaver.run();
					networksaver.run();
				}
				Poco::Thread::sleep(200);
				continue;
			}

			QuitNotification* pQuitNf = dynamic_cast<QuitNotification*>(pNf.get());
			if (pQuitNf)
				break;
		}
		else
		{
			break;
		}
	}
}


