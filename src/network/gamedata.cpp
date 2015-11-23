/*
 * gamedata.cpp
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include "gamedata.h"

#include "../other/language.h"
#include "../lix/lix_enum.h" // initialize strings
#include "../other/file/filename.h"
#include "../other/globals.h"

#include <ctime>
#include <string>
#include <sstream>
#include <Poco/Net/NetException.h>

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
