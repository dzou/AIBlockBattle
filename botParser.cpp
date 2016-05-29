#include <iostream>
#include <string>

#include "botState.h"
#include "botStarter.h"
#include "heuristic.h"

using namespace std;

extern float COEFF_ARR[];

#define FASTMODE true

class BotParser {
	public:
		void run() {
			BotStarter starter;
			BotState botState;
			string command = "";
			while (cin >> command) {
				if (command == "settings") {
					string p1, p2;
					cin >> p1 >> p2;
					botState.updateSetting(p1, p2);
				} else if (command == "update") {
					string p1, p2, p3;
					cin >> p1 >> p2 >> p3;
					botState.updateState(p1, p2, p3);
				} else if (command == "action") {
					starter.makeMove(botState, FASTMODE);
				} else {
					// cerr << "Unrecognized command: " << command << endl;
				}
			}
			// cout << botState.toString() << "\n";
		}
};

int main(int argc, char* argv[]) {
	assert(argc <= 13);
	for (int i = 1; i < argc; i++) {
		float num = atof(argv[i]);
		COEFF_ARR[i - 1] = num;
	}

	cerr << "My coefficients: ";
	for (int i = 0; i < 12; i++) {
		cerr << to_string(COEFF_ARR[i]) << ", ";
	}
	cerr << "\n";

	BotParser p;
	p.run();
	return 1;
}