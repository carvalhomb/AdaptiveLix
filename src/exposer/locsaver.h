/*
 * lsaver.h
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include <string>

class LocalSaver {

	public:
		LocalSaver(std::string received_payload);
		void save();
	private:
		std::string payload;
		static bool file_exists(std::string filename);
};

