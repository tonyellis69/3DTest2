#pragma once

#include "hex/hex.h"
#include "hex/hexArray.h"

/** A simple automaton that creates doorways between connected rooms. */

class CDoorBot {
public:
	CDoorBot(CHexArray& hexArray, CHex& startHex, CHex& endHex) : hexArray(hexArray), startHex(startHex), endHex(endHex) {
		currentHex = startHex;
	}
	void drawPos();
	void update();


	CHexArray& hexArray;
	CHex startHex;
	CHex endHex;
	CHex currentHex;
};



