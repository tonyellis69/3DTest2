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


	viewField.centre = CHex(0);
	viewField.setField(10);


}

/** Do the necessary one-off prep work for the given action. */
void CPlayerObject::initAction() {
	switch (action) {
	case tig::actPlayerMove: {
		/*CGetTravelPath pathRequest(hexPosition, targetHex);
		send(pathRequest);
		travelPath = pathRequest.travelPath;
		if (travelPath.size() > movePoints2)
			travelPath.resize(movePoints2);
		destHexClaimed = false;*/
		return;
	}

	case tig::actMelee: {
		;// animCycle = 0;
		return;
	}
	
	case tig::actShoot: {
		//CGetLineEnd msg(hexPosition,targetHex);
		//send(msg);
		//targetHex = msg.end;
		return;
	}



	}

}


void CPlayerObject::setActionMoveTo(CHex& hex) {
	action = tig::actMoveTo;
	targetHex = hex;
	CGetTravelPath pathRequest(hexPosition, targetHex);
	send(pathRequest);
	travelPath = pathRequest.travelPath;
	if (travelPath.size() > movePoints2)
		travelPath.resize(movePoints2);
	destHexClaimed = false;
	blockedFor = 0;
	CAddActor msg(this, actionSerial);
	send(msg);
}

bool CPlayerObject::update(float dT) {
	this->dT = dT;
	CHex oldPosition = hexPosition;

	switch (action) {
	case tig::actMoveTo:
	case tig::actPlayerMove:
		if (navigatePath(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actMelee:
		if (meleeAttack(dT)) {
			hitTarget();
			action = tig::actNone;
		}
		break;

	case tig::actShoot:
		if (shootTarget(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actNone:
		return resolved;
	}

	//TO DO: may not need as well as CActorMovedHex
	if (hexPosition != oldPosition) {
		CPlayerNewHex msg(hexPosition);
		send(msg);
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
	//for (auto hex : viewField.visibleHexes)
	//	hexRendr->highlightHex(hex);

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
	//TO DO: redundant, scrap
}

void CPlayerObject::onTakeItem(CTakeItem& msg) {
	takeItem(*msg.item);
}

void CPlayerObject::onGetPlayerObj(CGetPlayerObj& msg) {
	msg.playerObj =  this;
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

/** Called when player has arrived at a new hex.*/
void CPlayerObject::onMovedHex() {
	CActorMovedHex msg(hexPosition, this);
	send(msg);

	//update view field
	viewField.update(hexPosition);
	CCalcVisionField calcFieldMsg(hexPosition, viewField.ringHexes);
	send(calcFieldMsg);

	std::vector<CHex> unvisibledHexes;
	for (auto hex : viewField.visibleHexes) {
		if (std::find(calcFieldMsg.visibleHexes.begin(), calcFieldMsg.visibleHexes.end(),
			hex) == calcFieldMsg.visibleHexes.end()) {
			unvisibledHexes.push_back(hex);
		}
	}

	viewField.visibleHexes = calcFieldMsg.visibleHexes;

	//update fog of war
	CUpdateFog fogMsg(calcFieldMsg.visibleHexes,unvisibledHexes);
	send(fogMsg);

}


