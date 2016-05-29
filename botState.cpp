#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <cassert>

#include "bitField.h"
#include "shape.h"
#include "util.h"
#include "botState.h"

using namespace std;

void BotState::updateSetting(string key, string value) {
	if (key == "timebank") {
		timebank_ = stoi(value);
	} else if (key == "time_per_move") {
		timePerMove_ = stoi(value);
	} else if (key == "player_names") {
		vector<string> players = Split(value, ',');
		for (string name : players) {
			playerMap_[name] = Player(name); // unique_ptr<Player>(new Player(name));
		}
	} else if (key == "your_bot") {
		botName_ = value;
		if (botName_ == "player1") {
			oppName_ = "player2";
		} else {
			oppName_ = "player1";
		}
		assert(botName_ != oppName_);
	} else if (key == "field_width") {
		fieldWidth_ = stoi(value);
	} else if (key == "field_height") {
		fieldHeight_ = stoi(value);
	} else {
		cerr << "Command not recognized: \n";
		cerr << key << " " << value << "\n";
	}
}

void BotState::updateState(string playerName, string key, string value) {
	if (key == "round") {
		round_ = stoi(value);
	}	else if (key == "this_piece_type") {
		ShapeType type = getShapeType(value);
		currentShape_ = Shape(type);
	} else if (key == "next_piece_type") {
		ShapeType type = getShapeType(value);
		nextShape_ = Shape(type);
	} else if (key == "this_piece_position") {
		vector<string> tokens = Split(value, ',');
		assert(tokens.size() == 2);
		row_ = stoi(tokens[1]);
		col_ = stoi(tokens[0]);
	} else if (key == "row_points") {
		playerMap_[playerName].rowPoints = stoi(value); 	
	} else if (key == "combo") {
		playerMap_[playerName].combo = stoi(value);
	} else if (key == "skips") {
		playerMap_[playerName].skips = stoi(value);
	} else if (key == "field") {
		if (playerName == botName_) {
			myField_ = BitField(fieldWidth_, fieldHeight_, value);
		} else {
			oppField_ = BitField(fieldWidth_, fieldHeight_, value);
		}
	}
}

string BotState::toString() {
	string result = "";
	result += "Field Width: " + to_string(fieldWidth_) + "\n";
	result += "Field Height: " + to_string(fieldHeight_) + "\n";
	result += "Round: " + to_string(round_) + "\n";
	result += "Timebank: " + to_string(timebank_) + "\n";
	result += "Time Per Move: " + to_string(timePerMove_) + "\n";
	result += "Row: " + to_string(row_) + "\n";
	result += "Col: " + to_string(col_) + "\n";
	result += "Current Shape: \n" + currentShape_.toString() + "\n";
	result += "Next Shape: \n" + nextShape_.toString() + "\n";
	result += "Bot Name: " + botName_ + "\n";
	result += "Opp Name: " + oppName_ + "\n";
	for (auto const &kv: playerMap_) {
		result += kv.second.toString();
		result += "\n";
	}

	result += "My Field: \n";
	result += myField_.toString() + "\n";
	result += "Opp Field: \n";
	result += oppField_.toString() + "\n";

	return result;
}