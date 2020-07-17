#include "qps.h"

#include <string>


CQPS::CQPS() {
	messageBus.setHandler<CReserveNextPower>(this, &CQPS::onReserveNextPower);
	messageBus.setHandler<CFindPowerUser>(this, &CQPS::onFindPowerUser);
}

/** Remove any used power blobs, and top up with new ones. */
void CQPS::beginNewTurn() {
	int emptySlots = queueSize - powerQueue.size();
	for (int slot = 0; slot < emptySlots; slot++) {
		int power = spawnPower();
		powerQueue.push_back({ power,NULL });
	}

	refreshGUI();
}

/** Handle a request to reserve the next available power. */
void CQPS::onReserveNextPower(CReserveNextPower& msg) {
	for (auto slot : powerQueue)
		if (slot.reserver == msg.reserver)
			return;


	for (auto& slot : powerQueue) {
		if (slot.reserver == NULL) {
			slot.reserver = msg.reserver;
			break;
		}
	}
	refreshGUI();
}

/** Return the power assigned to this user, if found. If not found
	and the forced flag set, assign the next available power.*/
void CQPS::onFindPowerUser(CFindPowerUser& msg) {
	for (auto slot : powerQueue) {
		if (slot.reserver == msg.user) {
			msg.power = slot.power;
			return;
		}
	}

	if (msg.forced) {
		for (auto& slot : powerQueue) {
			if (slot.reserver == NULL) {
				slot.reserver = msg.user;
				msg.power = slot.power;
				break;
			}
		}
		refreshGUI();
	}
}

void CQPS::endTurn() {
	for (auto slot = powerQueue.begin(); slot != powerQueue.end();)
		if (slot->reserver) 
			slot = powerQueue.erase(slot);
		else
			slot++;
}

/** Return a newly generated power blob. */
int CQPS::spawnPower() {
	std::uniform_int_distribution<int> d{ 1,maxPower };
	return { d(randEngine) };
}

void CQPS::refreshGUI() {
	std::string txt;
	for (int slot = queueSize - 1; slot >= 0; slot--) {
		if (powerQueue[slot].reserver)
			txt += "\\style{reserved}";
		else
			txt += "\\style{default}";

		txt = txt + "\n" + std::to_string(powerQueue[slot].power);
		if (slot > 0)
			txt += "\n\n";
	}

	CSendText msg(powerQ, txt);
	send(msg);

}