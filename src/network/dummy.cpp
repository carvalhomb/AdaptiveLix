/*
 * dummy.cpp
 *
 *  Created on: 24 Nov 2015
 *      Author: maira
 */



#include "gameeventswrapper.h"
#include "dummy.h"

Dummy::Dummy():pocoThreadPool(NULL) {}
//Dummy::Dummy(GameData passed_data)
//{
//	data = passed_data;
//	pocoThreadPool(NULL);
//}

Dummy::~Dummy()
{
	if(pocoThreadPool != NULL)
	{
		delete pocoThreadPool;
		pocoThreadPool = NULL;
	}
}

void Dummy::start(GameData passed_data)
{
	data = passed_data;
	Poco::RunnableAdapter<Dummy> runnable(*this,&Dummy::run);
	if(pocoThreadPool==NULL) pocoThreadPool = new Poco::ThreadPool();
	pocoThreadPool->startWithPriority(Poco::Thread::PRIO_HIGHEST,runnable);
	pocoThreadPool->joinAll();
}

void Dummy::run()
{
	GameEventsWrapper eventswrapper(data);
	eventswrapper.run2();
}
