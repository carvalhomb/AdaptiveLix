/*
 * exposer.cpp
 *
 *  Created on: 24 Nov 2015
 *      Author: maira
 */

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning


#include <string>
#include <sstream>
#include <exception>

#include "../other/globals.h"
#include "../other/file/log.h"

#include <Poco/ThreadPool.h>

#include "locsaver.h"
#include "netsaver.h"

#include "exposer.h"

Exposer::Exposer() {}

Exposer::Exposer(GameData passed_data)
{
	data = passed_data;
}


void Exposer::run()
{
	NetworkSaver networksaver = NetworkSaver(data);
	LocalSaver localsaver = LocalSaver(data);
	Poco::ThreadPool::defaultPool().start(networksaver);
	Poco::ThreadPool::defaultPool().start(localsaver);
	Poco::ThreadPool::defaultPool().joinAll();
}


