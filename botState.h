#ifndef __BOT_STATE_H
#define __BOT_STATE_H

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "shape.h"
#include "bitField.h"

class Player {
	public:	
		std::string playerName;
		int rowPoints;
		int combo;
		int skips;

		Player() {
			playerName = "";
			rowPoints = 0;
			combo = 0;
			skips = 0;
		}

		Player(std::string name) {
			playerName = name;
			rowPoints = 0;
			combo = 0;
			skips = 0;
		}

		inline std::string toString() const {
			std::string result = "";
			result += "Player Name: " + playerName + "\n";
			result += "Row Points: " + std::to_string(rowPoints) + "\n";
			result += "Combo: " + std::to_string(combo) + "\n";
			result += "Skips: " + std::to_string(skips) + "\n";
			return result;
		}
};

class BotState {
	public:
		int fieldWidth_;
		int fieldHeight_;
		int round_;
		int timebank_;
		int timePerMove_;

		int row_;
		int col_;
		Shape currentShape_;
		Shape nextShape_;

		std::string botName_;
		std::string oppName_;
		std::map<std::string, Player> playerMap_;

		BitField myField_;
		BitField oppField_;

		BotState() 
		: fieldWidth_(10), 
			fieldHeight_(20), 
			currentShape_(ShapeType::NONE),
			nextShape_(ShapeType::NONE), 
			myField_(10, 20), 
			oppField_(10, 20) {}

		void updateSetting(std::string key, std::string value);
		void updateState(std::string v1, std::string v2, std::string v3);
		std::string toString();
};

#endif // __BOT_STATE_H