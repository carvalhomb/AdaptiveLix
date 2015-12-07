/*
 * lsaver.h
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include <string>
#include <Poco/RWLock.h>

#include "gamedata.h"

class LocalSaver {

	public:
		LocalSaver(GameData passed_event_data, std::string sessionid, Poco::RWLock* lock);
		void run();

	private:
		GameData event_data;
		std::string sessionid;
		void save_locally(std::string data_in_csv);
		static bool file_exists(std::string filename);
		Poco::RWLock* lock;
		bool replace(std::string& str, const std::string& from, const std::string& to);
};

