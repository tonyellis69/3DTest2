#pragma once

#include "hex/hexObject.h"

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CHexObject {
public:
	CRobot();
	void chooseTurnAction();
	void beginTurnAction();
	bool postAction();
	bool update(float dT);
	void receiveDamage(CHexObject& attacker, int damage);

private:
	int attackOrNot();


};


