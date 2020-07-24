#pragma once

#include "actor.h"

/** Encapsulates the functionality of multi-turn goals, such as 
	stalking the player or operating machinery.*/
class CGoalActor : public CHexActor {
public:
	CGoalActor() {}
	void setGoalWander();

	void chooseTurnAction2();


	int goal;
	CHex goalDestinationHex;
};