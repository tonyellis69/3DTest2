#pragma once

#include <vector>
#include <random>

#include "messaging/messenger.h"
#include "gameMsg.h"
#include "gamehextObj.h"


class CPowerRec {
public:
	int power;
	CGameHexObj* reserver = 0;
};

/** The Quantum Power System essentials. */
class CQPS : public CMessenger {
public:
	CQPS();
	void beginNewTurn();
	void onReserveNextPower(CReserveNextPower& msg);
	void onFindPowerUser(CFindPowerUser& msg);
	void endTurn();

private:
	int spawnPower();
	void refreshGUI();

	int maxPower = 10;
	int queueSize = 8;

	std::vector<CPowerRec> powerQueue;

	std::mt19937 randEngine;

};