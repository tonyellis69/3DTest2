#pragma once

#include "hex/hexObject.h"

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CHexObject {
public:
	void chooseTurnAction();
	bool update(float dT);




};