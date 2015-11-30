#pragma once

//#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>


#include "../gameplay/replay.h"
#include "../level/level.h"
#include "../other/user.h"

class GameData {

public:
	std::string level;
	Level levelobj;
	std::string action;
	signed long update;
	signed long seconds;
	signed long which_lix;
	std::string timestamp;
	int lix_required;
	int lix_saved;
	int skills_used;
	signed long seconds_required;

	GameData();
	GameData(std::string action, Level level, signed long update=-1);
	GameData(std::string action);
	void load_replay_data(Replay::Data data);
	void load_result_data(Result result);
	std::string to_csv();
//	std::string to_xml();
	std::string to_json();

private:
	std::string extract_action_word(Replay::Data data);
	std::string get_timestamp();
};


