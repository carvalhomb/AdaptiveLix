/*
 * lsaver.cpp
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>
#include <algorithm>
#include <sstream>
#include <exception>

#include <Poco/RWLock.h>

#include "../other/file/log.h"
#include "../other/globals.h"

#include "locsaver.h"

using namespace std;

LocalSaver::LocalSaver(GameData passed_event_data, string _sessionid, Poco::RWLock* _lock) {
	event_data = passed_event_data;
	lock = _lock;
	sessionid = _sessionid;
}

void LocalSaver::run() {
	if (gloB->exposer_offline_mode == true or gloB->exposer_record_local_file==true) {
		//format data to csv
		string data_in_csv = event_data.to_csv();
		save_locally(data_in_csv);
	}
}

void LocalSaver::save_locally(string data_in_csv) {
	try {
		string filename = gloB->exposer_local_output.get_rootful();
		ofstream myfile;

		//Replace %sessionid% for the real sessionid
		//replace(data_in_csv, "%sessionid%", sessionid);

		lock->writeLock(); //acquire lock

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
			myfile << "timestamp,action,level,update,";
			myfile << "which_lix,lix_required,lix_saved,";
			myfile << "skills_used,seconds_required,seconds_used \n";
			myfile << data_in_csv;
			myfile.close();
		}

		lock->unlock();  //release lock
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

bool LocalSaver::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
