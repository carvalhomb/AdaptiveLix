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

GameData::GameData(string action, Level level, signed long int update) {
	this->timestamp = GameData::get_timestamp();
	this->action=action;
	this->which_lix=-1;
	this->update=update;
	this->seconds=-1;
	this->lix_required=0;
	this->lix_saved=0;
	this->skills_used=0;
	this->seconds_required=0;

	this->levelobj = level;
	this->level = level.level_filename;

	if (this->update>0) {
		signed long secs = this->update / gloB->updates_per_second;
		this->seconds = secs;
	}
}

GameData::GameData(string action) {
	this->timestamp = GameData::get_timestamp();
	this->action=action;
	this->which_lix=-1;
	this->update=-1;
	this->seconds=-1;
	this->lix_required=0;
	this->lix_saved=0;
	this->skills_used=0;
	this->seconds_required=0;
	this->levelobj = Level();
	this->level = "0"; //Not in a level
}

GameData::GameData() {
	this->timestamp = GameData::get_timestamp();
	this->action="0";
	this->which_lix=-1;
	this->update=-1;
	this->seconds=-1;
	this->lix_required=0;
	this->lix_saved=0;
	this->skills_used=0;
	this->seconds_required=0;
	this->levelobj = Level();
	this->level = "0"; //Not in a level
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
	this->update = data.update;
	signed long secs = this->update / gloB->updates_per_second;
	this->seconds = secs;

	this->action = GameData::extract_action_word(data);

	if (data.action == Replay::ASSIGN || data.action == Replay::ASSIGN_LEFT
				|| data.action == Replay::ASSIGN_RIGHT) {
			this->which_lix = data.what;
	}

}


void GameData::load_result_data(Result result) {
	this->action = "ENDLEVEL";
	this->update = -1;
	this->which_lix = -1;

	if (this->levelobj.get_good()==true) {
		this->lix_required = this->levelobj.required;
		this->seconds_required=this->levelobj.seconds;
	}

	this->lix_saved = result.lix_saved;
	this->skills_used = result.skills_used;

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
		this->which_lix = data.what;
	}

	if (data.action == Replay::SPAWNINT ) {
		action_word_stream << "=";
		action_word_stream << data.what;
	}

	return action_word_stream.str();
}

string GameData::to_xml()
{
	ostringstream data_sstr;

	data_sstr << "<event>";
	data_sstr << "<timestamp>" << timestamp << "</timestamp>";
	data_sstr << "<action>" << action << "</action>";
	if (level != "0") data_sstr <<  "<level>" << level << "</level>";
	if (update >= 0) data_sstr << "<update>" << update << "</update>";
	if (seconds >= 0) data_sstr << "<seconds>" << seconds << "</seconds>";
	if (which_lix >= 0 ) data_sstr << "<which_lix>" << which_lix << "</which_lix>";
	if (lix_required >= 0 && (action == "ENDLEVEL")) data_sstr << "<lix_required>" << lix_required << "</lix_required>";
	if (lix_saved >= 0 && (action == "ENDLEVEL")) data_sstr << "<lix_saved>" << lix_saved << "</lix_saved>";
	if (skills_used >= 0 && (action == "ENDLEVEL")) data_sstr << "<skills_used>" << skills_used << "</skills_used>";
	if (seconds_required >= 0 && (action == "ENDLEVEL")) data_sstr << "<seconds_required>" << seconds_required << "</seconds_required>";
	data_sstr << "</event>";
	return data_sstr.str();
}

string GameData::to_json()
{
	ostringstream data_sstr;

	data_sstr << "{ ";
	data_sstr << "\"timestamp\" : \"" << timestamp << "\",\n";
	data_sstr << "\"action\" : \"" << action << "\",\n";
	data_sstr <<  "\"level\" : \"" << level << "\",\n";
	data_sstr << "\"update\" : \"" << update << "\",\n";
	data_sstr << "\"seconds\" : \"" << seconds << "\",\n";
	data_sstr << "\"which_lix\" : \"" << which_lix << "\",\n";
	data_sstr << "\"lix_required\" : \"" << lix_required << "\",\n";
	data_sstr << "\"lix_saved\" : \"" << lix_saved << "\",\n";
	data_sstr << "\"skills_used\" : \"" << skills_used << "\",\n";
	data_sstr << "\"seconds_required\" : \"" << seconds_required << "\"\n";
	data_sstr << "}";
	return data_sstr.str();
}

string GameData::to_csv()
{
	ostringstream data_sstr;

	data_sstr << gloB->exposer_sessionid << ", ";
	data_sstr << timestamp << ", ";
	data_sstr << action << ", ";
	data_sstr << level << ", ";
	data_sstr << update << ", ";
	data_sstr << seconds << ", ";
	data_sstr << which_lix << ", ";
	data_sstr << lix_required << ", ";
	data_sstr << lix_saved << ", ";
	data_sstr << skills_used << ", ";
	data_sstr << seconds_required << "\n";
	return data_sstr.str();
}
