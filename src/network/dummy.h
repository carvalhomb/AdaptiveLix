/*
 * dummy.h
 *
 *  Created on: 23 Nov 2015
 *      Author: maira
 */

#include <Poco/RunnableAdapter.h>
#include <Poco/ThreadPool.h>

#include "gamedata.h"

class Dummy
{
public:
	Dummy();
	~Dummy();
	void start(GameData passed_data);
	void run();
	GameData data;
	Poco::ThreadPool* pocoThreadPool;
};
