/*
 * lsaver.h
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include <string>

#include "gamedata.h"
#include <Poco/Runnable.h>

class LocalSaver : public Poco::Runnable {

	public:
		LocalSaver(GameData passed_event_data);
		virtual void run();
	private:
		GameData event_data;
		void save_locally(std::string data_in_csv);
		static bool file_exists(std::string filename);
};

