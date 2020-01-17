#pragma once

#include "hex/hexObject.h"

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CHexObject {
public:
	CRobot();
	void chooseTurnAction();
	void beginTurnAction();
	void beginTurnToPlayer();
	void onEndOfAction();
	bool update(float dT);


private:



};


