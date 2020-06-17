#pragma once

#include <vector>
#include <random>

#include "gamehextObj.h"

#include "powerUser.h"



class CDefenceRec;
enum TPowerAssignee {assigneeNone, assigneeGun, assigneeDefence, assigneeTmp};
struct TPowerBlob {
	int power;
	TPowerAssignee reservedBy = assigneeNone;
	unsigned int id;
};

/** Supplies packets of quantum energy to the player's equipment. */
class CPowerSupply {
public:
	CPowerSupply();
	void topUp();
	void updateDisplay();
	int readNextPower();
	unsigned int reserveNextPower();
	int getPowerValue(unsigned int id);
	void onTurnEnd();
	void unreservePower(unsigned int power);
	int consumePower(unsigned int id);
	void requestPower(CPowerUser* user);
	void removeRequest(CPowerUser* user);
	unsigned int getBestPower(CPowerUser* user);
	void autoAssign();

	int numSlots;
	int maxPower; ///<Greatest energy pack createable.


private:
	//std::vector<int> powerSlots;
	std::vector<TPowerBlob> powerSlots;
	std::vector<int> returnedPower;

	std::mt19937 randEngine;

	unsigned int nextId;

	std::vector<CPowerUser*> powerRequests;
};