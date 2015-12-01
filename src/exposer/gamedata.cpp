/*
 * gamedata.cpp
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include "../other/language.h"
#include "../lix/lix_enum.h" // initialize strings
#include "../other/file/filename.h"
#include "../other/globals.h"

#include <ctime>
#include <string>
#include <sstream>
#include <exception>
#include <Poco/Net/NetException.h>

#include "gamedata.h"

using namespace std;

GameData::GameData(string passed_action, Level passed_level, signed long passed_update) {
	timestamp = GameData::get_timestamp();
	action=passed_action;
	which_lix=-1;
	update=passed_update;
	lix_required=0;
	lix_saved=0;
	skills_used=0;
	seconds_required=0;
	seconds_used=0;

	levelobj = passed_level;
	level = passed_level.level_filename;

//	if (update>0) {
//		signed long secs = update / gloB->updates_per_second;
//		seconds_used = secs;
//	}
}

GameData::GameData(string passed_action) {
	timestamp = GameData::get_timestamp();
	action=passed_action;
	which_lix=-1;
	update=-1;
	lix_required=0;
	lix_saved=0;
	skills_used=0;
	seconds_required=0;
	seconds_used=0;
	levelobj = Level();
	level = ""; //Not in a level
}

GameData::GameData() {
	timestamp = GameData::get_timestamp();
	action="";
	which_lix=-1;
	update=-1;
	seconds_used=0;
	lix_required=0;
	lix_saved=0;
	skills_used=0;
	seconds_required=0;
	levelobj = Level();
	level = ""; //Not in a level
}

string GameData::get_timestamp(){
	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];
	ostringstream timestampstream;
	time ( &rawtime );
	timeinfo = gmtime ( &rawtime );
	strftime(buffer,80,"%Y-%m-%dT%H:%M:%S",timeinfo);
	timestampstream << buffer;
	std::string timestampstring = timestampstream.str();
	timestampstring += "Z"; //Add UTC as timezone, since we used gmttime
	return timestampstring;
}

void GameData::load_replay_data(Replay::Data data) {
	update = data.update;

	action = GameData::extract_action_word(data);

	if (data.action == Replay::ASSIGN || data.action == Replay::ASSIGN_LEFT
				|| data.action == Replay::ASSIGN_RIGHT) {
			which_lix = data.what;
	}

}


void GameData::load_result_data(Result result) {
	action = "RESULT";
	update = -1;
	which_lix = -1;

	if (levelobj.get_good()==true) {
		lix_required = levelobj.required;
		seconds_required=levelobj.seconds;
		update = result.updates_used;
	}

	lix_saved = result.lix_saved;
	skills_used = result.skills_used;

	if (update>0) {
		signed long secs = update / gloB->updates_per_second;
		seconds_used = secs;
	}

}


string GameData::extract_action_word(Replay::Data data)
{
	//Convert info in replay_data object into words to send to service
	string action_word;
	action_word = data.action == Replay::SPAWNINT ? gloB->replay_spawnint
			: data.action == Replay::NUKE ? gloB->replay_nuke
					: data.action == Replay::ASSIGN ? gloB->replay_assign_any
							: data.action == Replay::ASSIGN_LEFT  ? gloB->replay_assign_left
									: data.action == Replay::ASSIGN_RIGHT ? gloB->replay_assign_right
											: Language::common_cancel;

	ostringstream action_word_stream;
	action_word_stream << action_word;
	if (data.action == Replay::ASSIGN || data.action == Replay::ASSIGN_LEFT
			|| data.action == Replay::ASSIGN_RIGHT) {
		action_word_stream << "=";
		action_word_stream << LixEn::ac_to_string(static_cast <LixEn::Ac> (data.skill));
		which_lix = data.what;
	}

	if (data.action == Replay::SPAWNINT ) {
		action_word_stream << "=";
		action_word_stream << data.what;
	}

	return action_word_stream.str();
}

//string GameData::to_xml()
//{
//	string which_lix_string;
//	string update_string;
//	if (which_lix == -1) which_lix_string = "";
//	if (update == -1) update_string = "";
//
//	ostringstream data_sstr;
//
//	data_sstr << "\"<event>";
//	data_sstr << "<timestamp>" << timestamp << "</timestamp>";
//	data_sstr << "<action>" << action << "</action>";
//	data_sstr << "<level>" << level << "</level>";
//	data_sstr << "<update>" << update_string << "</update>";
//	data_sstr << "<which_lix>" << which_lix_string << "</which_lix>";
//	data_sstr << "<result>";
//	if (action == "RESULT") {
//		data_sstr << "<element>";
//		data_sstr << "<lix_required>" << lix_required << "</lix_required>";
//		data_sstr << "<lix_saved>" << lix_saved << "</lix_saved>";
//		data_sstr << "<seconds_required>" << seconds_required << "</seconds_required>";
//		data_sstr << "<seconds_used>" << seconds << "</seconds_used>";
//		data_sstr << "<skills_used>" << skills_used << "</skills_used>";
//		data_sstr << "</element>";
//	}
//	data_sstr << "</result>";
//	data_sstr << "</event>\"";
//	return data_sstr.str();
//}

string GameData::to_json()
{

	//	string which_lix_string;
	//	string update_string;
	//	if (which_lix == -1) which_lix_string = "";
	//	if (update == -1) update_string = "";

	ostringstream which_lix_string;
	ostringstream update_string;
	ostringstream lix_saved_string;
	ostringstream lix_required_string;
	ostringstream seconds_required_string;
	ostringstream seconds_used_string;
	ostringstream skills_used_string;
	if (which_lix == -1) {
		which_lix_string << "";
	} else {
		which_lix_string << which_lix;
	}

	if (update == -1) {
		update_string << "";
	} else {
		update_string << update;
	}
	if (action != "RESULT") {
		lix_saved_string << "";
		lix_required_string << "";
		seconds_required_string << "";
		skills_used_string << "";
		seconds_used_string << "";
	} else {
		lix_saved_string << lix_saved;
		lix_required_string << lix_required;
		seconds_required_string << seconds_required;
		skills_used_string <<skills_used;
		seconds_used_string << seconds_used;
	}

	ostringstream data_sstr;

	data_sstr << "[{ ";
	data_sstr << "\"timestamp\" : \"" << timestamp << "\",\n";
	data_sstr << "\"action\" : \"" << action << "\",\n";
	data_sstr <<  "\"level\" : \"" << level << "\",\n";
	data_sstr << "\"update\" : \"" << update_string.str() << "\",\n";
	data_sstr << "\"which_lix\" : \"" << which_lix_string.str() << "\",\n";
	data_sstr << "\"result\" : [";
	if (action=="RESULT") {
		data_sstr << "{ \"lix_required\" : \"" << lix_required_string.str() << "\",\n";
		data_sstr << "\"lix_saved\" : \"" << lix_saved_string.str() << "\",\n";
		data_sstr << "\"seconds_required\" : \"" << seconds_required_string.str() << "\",\n";
		data_sstr << "\"seconds_used\" : \"" << seconds_used_string.str() << "\",\n";
		data_sstr << "\"skills_used\" : \"" << skills_used_string.str() << "\"}\n";
	}
	data_sstr << "] \n}]";
	return data_sstr.str();
}

string GameData::to_csv()
{

	ostringstream which_lix_string;
	ostringstream update_string;
	ostringstream lix_saved_string;
	ostringstream lix_required_string;
	ostringstream seconds_required_string;
	ostringstream skills_used_string;
	ostringstream seconds_used_string;
	if (which_lix == -1) {
		which_lix_string << "";
	} else {
		which_lix_string << which_lix;
	}

	if (update == -1) {
		update_string << "";
	} else {
		update_string << update;
	}
	if (action != "RESULT") {
		lix_saved_string << "";
		lix_required_string << "";
		seconds_required_string << "";
		skills_used_string << "";
		seconds_used_string << "";
	} else {
		lix_saved_string << lix_saved;
		lix_required_string << lix_required;
		seconds_required_string << seconds_required;
		skills_used_string <<skills_used;
		seconds_used_string << seconds_used;
	}

	ostringstream data_sstr;

	data_sstr << gloB->exposer_sessionid << ", ";
	data_sstr << timestamp << ", ";
	data_sstr << action << ", ";
	data_sstr << level << ", ";
	data_sstr << update_string.str() << ", ";
	data_sstr << which_lix_string.str() << ", ";
	data_sstr << lix_required_string.str() << ", ";
	data_sstr << lix_saved_string.str() << ", ";
	data_sstr << skills_used_string.str() << ", ";
	data_sstr << seconds_required_string.str() << ", ";
	data_sstr << seconds_used_string.str() << "\n";
	return data_sstr.str();
}
