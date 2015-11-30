/*
 * exposer.h
 *
 *  Created on: 23 Nov 2015
 *      Author: maira
 */

#include <Poco/NotificationQueue.h>

#include "gamedata.h"

class Exposer
{
public:
	Exposer(GameData passed_data, Poco::NotificationQueue* nq);
	void run();

private:
	GameData _data;
	Poco::NotificationQueue* _nq;
};


