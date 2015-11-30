/*
 * lsaver.cpp
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>
#include <sstream>
#include <exception>

#include "../other/file/log.h"
#include "../other/globals.h"

#include "locsaver.h"

using namespace std;

LocalSaver::LocalSaver(GameData passed_event_data) {
	event_data = passed_event_data;
}

void LocalSaver::run() {
	if (gloB->exposer_offline_mode == true or gloB->exposer_record_local_file==true) {
		//format data to csv
		std::string data_in_csv = event_data.to_csv();
		save_locally(data_in_csv);
	}
}

void LocalSaver::save_locally(string data_in_csv) {
	try {
		string filename = gloB->exposer_local_output.get_rootful();
		ofstream myfile;

		if (file_exists(filename)) {
			//Log::log(Log::INFO, "File exists, appending...");
			myfile.open(filename.c_str(), std::ios_base::app);
			myfile << data_in_csv;
			myfile.close();
		}
		else {
			//Log::log(Log::INFO, "File does NOT exists, creating it and preparing header...");

			//create file and write first line, then the formatted line
			myfile.open(filename.c_str());
			myfile << "sessionid, timestamp, action, level, update, ";
			myfile << "which_lix, lix_required, lix_saved, ";
			myfile << "skills_used, seconds_required \n";
			myfile << data_in_csv;
			myfile.close();
		}
	}
	catch (std::exception &ex) {
		ostringstream tmpmsg;
		tmpmsg << "Exception while trying to write local file. " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
	}
}

bool LocalSaver::file_exists(std::string filename) {
    ifstream f(filename.c_str(), ifstream::in);
    return !f.fail(); // using good() could fail on empty files
}


