#include "playerHexObj.h"

#include "utils/log.h"

#include "hexItem.h"



CPlayerObject::CPlayerObject() {
	psu = new CPowerSupply();

	messageBus.setHandler<CGetPlayerPos>(this, &CPlayerObject::onGetPlayerPos);
	messageBus.setHandler<CSetPlayerAction>(this, &CPlayerObject::onSetPlayerAction);
	messageBus.setHandler<CTakeItem>(this, &CPlayerObject::onTakeItem);
	messageBus.setHandler<CGetPlayerObj>(this, &CPlayerObject::onGetPlayerObj);

	tmpHP = 12;
}

/** Do the necessary one-off prep work for the given action. */
void CPlayerObject::initAction() {
	switch (action) {
	case tig::actPlayerMove: {
		CGetTravelPath pathRequest(hexPosition, targetHex);
		send(pathRequest);
		travelPath = pathRequest.travelPath;
		if (travelPath.size() > movePoints2)
			travelPath.resize(movePoints2);
		destHexClaimed = false;
		return;
	}

	case tig::actPlayerMeleeAttack: {
		animCycle = 0;
		return;
	}
	
	case tig::actPlayerShoot: {
		CGetLineEnd msg(hexPosition,targetHex);
		send(msg);
		targetHex = msg.end;
		return;
	}



	}

}

bool CPlayerObject::update(float dT) {
	this->dT = dT;
	switch (action) {
	case tig::actPlayerMove:
		if (navigatePath(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actPlayerMeleeAttack:
		if (meleeAttack(dT)) {
			hitTarget();
			action = tig::actNone;
		}
		break;

	case tig::actPlayerShoot:
		if (shootTarget(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actNone:
		return resolved;



	}

	return unresolved;
}





/** Deliver melee damage to our current target. */
void CPlayerObject::hitTarget() {
	if (actionTarget == NULL)
		return;

	CFindPowerUser msg(this);
	send(msg);
	int damage = msg.power;

	actionTarget->receiveDamage(*this, damage);
}



void CPlayerObject::draw() {
	//draw self
	CHexObject::draw();
}


void CPlayerObject::takeItem(CGameHexObj& item) {

	CRemoveEntity msg(&item);
	send(msg);

	playerItems.push_back(&item);

	callTig(tig::onTake, item);
}

void CPlayerObject::showInventory() {
	callTig(tig::onInventory);
}

void CPlayerObject::dropItem(int itemNo) {	
	ITigObj* item = callTigObj(tig::onDrop, itemNo);
	CGameHexObj* gameObj = (CGameHexObj*)item->getCppObj();
	auto playerItem = std::find(playerItems.begin(), playerItems.end(), gameObj);

	CDropItem msg((CHexItem *) *playerItem, hexPosition);
	send(msg);
	playerItems.erase(playerItem);
}

void CPlayerObject::equipItem(int itemNo) {
	callTigObj(tig::onEquip, itemNo);
}

/** Respond to left-click/primary action in power mode. */
void CPlayerObject::leftClickPowerMode() {
	CReserveNextPower msg(this);
	send(msg);
}


void CPlayerObject::onTurnBegin() {
	liveLog << "\nPlayer turn begin";
	psu->topUp();

	psu->updateDisplay();
}

/**Tidy up player state ready for the next round. */
void CPlayerObject::onTurnEnd() {

	psu->onTurnEnd();
	psu->updateDisplay();
}

void CPlayerObject::onGetPlayerPos(CGetPlayerPos& msg) {
	msg.position = hexPosition;
}

void CPlayerObject::onSetPlayerAction(CSetPlayerAction& msg) {
	if (msg.targetObj)
		setAction(msg.action, msg.targetObj);
	else
		setAction(msg.action, msg.targetHex);
}

void CPlayerObject::onTakeItem(CTakeItem& msg) {
	takeItem(*msg.item);
}

void CPlayerObject::onGetPlayerObj(CGetPlayerObj& msg) {
	msg.playerObj = this;
}

void CPlayerObject::deathRoutine() {
	std::string deathLog = "\nYou were killed!";

	CSendText msg(combatLog, deathLog);
	send(msg);

	CKill killMsg(this);
	send(killMsg);
}

void CPlayerObject::receiveDamage(CGameHexObj& attacker, int damage) {
	CFindPowerUser msg(&attacker);
	send(msg);

	damage -= msg.power;

	if (damage <= 0) {
		CSendText block(combatLog, "\nAttack from " + attacker.getName()
			+ " blocked!");
		send(block);
		return;
	}

	CGameHexObj::receiveDamage(attacker, damage);
}

int CPlayerObject::getMissileDamage() {
	//get assigned power from QPS
	CFindPowerUser msg(this);
	send(msg);

	return msg.power;
}


