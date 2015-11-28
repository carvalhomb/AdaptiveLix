/*
 * exposer.h
 *
 *  Created on: 23 Nov 2015
 *      Author: maira
 */

//#include <Poco/RunnableAdapter.h>
//#include <Poco/ThreadPool.h>
#include <Poco/NotificationCenter.h>

#include "gamedata.h"

class Exposer
{
public:
//	Exposer();
	Exposer(GameData passed_data, Poco::NotificationCenter* nc);
//	Exposer(GameData passed_data);
	void run();

private:
	GameData data;
	Poco::NotificationCenter* nc;
};


