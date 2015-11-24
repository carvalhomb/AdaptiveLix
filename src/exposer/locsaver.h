/*
 * lsaver.h
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include <string>


#include <Poco/Runnable.h>

class LocalSaver : public Poco::Runnable {

	public:
		LocalSaver(std::string received_payload);
		//void save();
		virtual void run();
	private:
		std::string payload;
		static bool file_exists(std::string filename);
};

