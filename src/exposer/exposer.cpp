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

#include "locsaver.h"
#include "netsaver.h"

#include "exposer.h"

Exposer::Exposer() {}

Exposer::Exposer(GameData passed_data)
{
	data = passed_data;
}

Exposer::~Exposer()
{

}



void Exposer::run()
{
	//Is the game running in offline mode?
	if (gloB->exposer_offline_mode == false) {
		NetworkSaver networksaver = NetworkSaver(data);
		networksaver.save();
	}

	//Is the game set to record data locally or is it running offline?
	if (gloB->exposer_offline_mode == true or gloB->exposer_record_local_file==true) {
		//format data to csv
		std::string data_in_csv = data.to_csv();
		LocalSaver localsaver = LocalSaver(data_in_csv);
		localsaver.save();
	}
}


