/*
 * exposer.h
 *
 *  Created on: 23 Nov 2015
 *      Author: maira
 */

//#include <Poco/RunnableAdapter.h>
//#include <Poco/ThreadPool.h>

#include "gamedata.h"

class Exposer
{
public:
	Exposer();
	Exposer(GameData passed_data);
	~Exposer();
	void run();
private:
	GameData data;
};


