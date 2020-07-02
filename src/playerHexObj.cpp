#include "playerHexObj.h"

#include "utils/log.h"

#include "hexItem.h"

#include "IHexWorld.h" //NB can scrap once item handling tidied to use messages



CPlayerObject::CPlayerObject() {
	psu = new CPowerSupply();

	messageBus.setHandler<CGetPlayerPos>(this, &CPlayerObject::onGetPlayerPos);
	messageBus.setHandler<CSetPlayerAction>(this, &CPlayerObject::onSetPlayerAction);
	messageBus.setHandler<CTakeItem>(this, &CPlayerObject::onTakeItem);
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
		animCycle2 = 0;
		return;
	}
	
	case tig::actPlayerShoot: {
		CGetLineEnd msg(hexPosition,targetHex);
		send(msg);
		targetHex = msg.end;// map->findLineEnd(hexPosition, targetHex);
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
			return resolved;
		}
		else return unresolved;

	case tig::actPlayerMeleeAttack:
		if (meleeAttack(dT)) {
			hitTarget();
			action = tig::actNone;
			return resolved;
		}
		else return unresolved;

	case tig::actPlayerShoot:
		if (shootTarget(dT)) {
			hitTarget();
			action = tig::actNone;
			return resolved;
		}
		else return unresolved;

	case tig::actNone:
		return resolved;



	}

	return resolved;
}





/** Deliver melee damage to our current target. */
void CPlayerObject::hitTarget() {
	/*int weaponDamage = callTigInt(tig::getWeaponDamage);
	attackTarget->receiveDamage(*this, weaponDamage);*/

	liveLog << "\nPlayer lunged ";
}


void CPlayerObject::receiveDamage(CGameHexObj& attacker, int damage) {
	THexDir attackDir = neighbourDirection(hexPosition, attacker.hexPosition);

	callTig(tig::onReceiveDamage, attacker, damage);
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
	setAction(msg.action, msg.target);
}

void CPlayerObject::onTakeItem(CTakeItem& msg) {
	takeItem(*msg.item);
}




