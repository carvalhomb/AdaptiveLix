/*
 * lsaver.h
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include <string>

#include "gamedata.h"

class LocalSaver {

	public:
		LocalSaver(GameData passed_event_data);
		void run();
	private:
		GameData event_data;
		void save_locally(std::string data_in_csv);
		static bool file_exists(std::string filename);
};

