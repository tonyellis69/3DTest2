#pragma once

#include "gamehextObj.h"

/** A class encapuslating the movement code for game entities. */
class CHexActor : public CGameHexObj {
public:
	void startAction(int action);
	bool update2(float dT);


protected:
	bool navigatePath(float dT);


//low level funcs
	bool isFacing(CHex& hex);

	float dT;
	int action;
	THexList travelPath2; ///<A sequence of hexes to travel down.
};