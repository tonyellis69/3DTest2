#include "playerHexObj.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/integer.hpp>

#include "utils/log.h"

#include "IHexWorld.h"

#include "bolt.h"
#include "IGameHexArray.h"

CPlayerObject::CPlayerObject() {
	fireable = 0;

	tmpShield = new CShield("Shield");
	tmpWeapon = new CWeapon("Gun");
	fireables.push_back(tmpShield);
	fireables.push_back(tmpWeapon);


	psu = new CPowerSupply();
	CFireable::psu = psu;
}

/** Push the given action onto the player object's stack, with any supporting actions. */
void CPlayerObject::stackAction(CAction chosenAction) {
	currentAction.actionId = tig::actNone;
	switch (chosenAction.actionId) {
	case tig::actPlayerMove :
			actions.push({ tig::actPlayerMove, NULL });
			return;
	case tig::actPlayerMeleeAttack :
			actions.push({ tig::actPlayerMeleeAttack, chosenAction.object });
			actions.push({ tig::actTurnToTarget, chosenAction.object });
			return;
	case tig::actPlayerShoot :
			actions.push({ tig::actPlayerShoot, chosenAction.object });
			actions.push({ tig::actTurnToTarget, chosenAction.object });
			return;
	}
}

bool CPlayerObject::update(float dT) {
	if (currentAction.actionId == tig::actNone) {
		if (actions.empty())
			return false; 
		currentAction = actions.top();
		actions.pop();
		initialiseCurrentAction();
	}

	bool resolving = false;

	if (currentAction.actionId == tig::actTurnToTarget)
		resolving = updateRotationOnly(dT);

	if (currentAction.actionId == tig::actPlayerMove)
		resolving = updateMove(dT);

	if (currentAction.actionId == tig::actPlayerMeleeAttack) {
		resolving = updateLunge(dT);
	}


	if (!resolving) {
		currentAction.actionId = tig::actNone;
		if (actions.empty())
			return false;
	}

	return true;


}

/** Initiate a player lunge attack on the target. */
void CPlayerObject::beginLunge(CGameHexObj& target) {
	attackTarget = &target;
	tigMemberInt(tig::action) = tig::actPlayerMeleeAttack;
	if (initTurnToAdjacent(target.hexPosition))
		tigMemberInt(tig::action) = tig::actPlayerTurnToAttack;

	animCycle = 0;
	moveVector = directionToVec(destinationDirection);
	hexWorld->addToSerialActions(this);
}


/** Deliver melee damage to our current target. */
void CPlayerObject::hitTarget() {
	int weaponDamage = callTigInt(tig::getWeaponDamage);
	attackTarget->receiveDamage(*this, weaponDamage);

	liveLog << "\nLunged " << std::to_string(tmpWeapon->powerId);
}


void CPlayerObject::receiveDamage(CGameHexObj& attacker, int damage) {
	THexDir attackDir = neighbourDirection(hexPosition, attacker.hexPosition);



	if (tmpShield->findDefendee(&attacker)) {
		liveLog << "\nShield blocks attack.";
		return;
	}

	callTig(tig::onReceiveDamage, attacker, damage);

}

void CPlayerObject::draw() {

	

	//draw self
	CHexObject::draw();

	//draw shield:
	int action = tigMemberInt(tig::action);
	if (action ==  tig::actPlayerMeleeAttack)
		return;

	for (int face = 0; face < 6; face++) {
		if (shields[face] == 0)
			continue;
		float rotation = face * -60;
		glm::mat4 rot = glm::rotate(*worldMatrix, glm::radians(rotation), glm::vec3(0, 0, 1));
		shieldModel.model.matrix = rot;
		hexRendr->drawLineModel(shieldModel);
	}
}


void CPlayerObject::setShield(THexDir shieldDir) {
	if (tigMemberInt(tig::equippedShield) == 0) {
		;// return;
	}

	int shieldNo = (shieldDir - facing) % 6;
	if (shieldNo < 0)
		shieldNo += 6;

	shields[shieldNo] = !shields[shieldNo];
}


void CPlayerObject::takeItem(CGameHexObj& item) {
	hexWorld->removeEntity(item);
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
	hexWorld->dropItem((*playerItem), hexPosition);
	playerItems.erase(playerItem);
}

void CPlayerObject::equipItem(int itemNo) {
	callTigObj(tig::onEquip, itemNo);
}

void CPlayerObject::fireShot(CHex& target) {
	CHex endHex = map->findLineEnd(hexPosition, target);
	CBolt* boltTmp = (CBolt*)hexWorld->createBolt();
	boltTmp->setPosition(hexPosition);
	boltTmp->fireAt(endHex);

	liveLog << "\nFired " << std::to_string(tmpWeapon->powerId);
}

void CPlayerObject::cycleFireable(float delta) {
	fireable += int(delta);
	fireable %= fireables.size();
}

/** Fire current fireable at target. */
bool CPlayerObject::fireAt(CGameHexObj* target) {
	return fireables[fireable]->fireAt(target);
}

void CPlayerObject::onTurnBegin() {
	liveLog << "\nPlayer turn begin";
	psu->topUp();
	if (tmpWeapon->autoPower == autoCurrent && tmpWeapon->powerId == 0) {
		tmpWeapon->powerId = 0; //was psu->reserveNextPower();
		//liveLog << "\ngun takes " << std::to_string(psu->getPowerValue(tmpWeapon->powerId));
	}

	//ditto for shield
	for (auto& defence : tmpShield->defences) {
		if (defence.second.autoPower == autoCurrent) {
			defence.second.powerId = 0; //was psu->reserveNextPower();
			//defence.second.power = psu->getPowerValue(defence.second.powerId);
			//liveLog << "\nshield takes " << std::to_string(psu->getPowerValue(defence.second.powerId));
		}

	}

	psu->updateDisplay();
}

/**Tidy up player state ready for the next round. */
void CPlayerObject::onTurnEnd() {
	tmpWeapon->onTurnEnd();
	tmpShield->onTurnEnd();

	psu->onTurnEnd();
	psu->updateDisplay();
}

void CPlayerObject::initialiseCurrentAction() {
	switch (currentAction.actionId) {
	case tig::actPlayerMove:	movePoints = 20;
								setTravelPath(*hexWorld->getCursorPath());
								beginMove(); break;
		case tig::actPlayerMeleeAttack: beginLunge(*currentAction.object); break;
		case tig::actTurnToTarget: beginTurnToTarget(currentAction.object->hexPosition); break;
		case tig::actPlayerShoot: fireShot(currentAction.object->hexPosition); break;
	}
}


