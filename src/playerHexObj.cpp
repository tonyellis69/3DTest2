#include "playerHexObj.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/integer.hpp>

#include "utils/log.h"

#include "IHexWorld.h"

#include "bolt.h"
#include "IGameHexArray.h"

CPlayerObject::CPlayerObject() {
	//rotation = 45;
}

bool CPlayerObject::update(float dT) {
	bool resolving = updateMove(dT);

	int action = getChosenAction();
	if (action ==  tig::actPlayerTurnToAttack && !turning) {
		moving = false;
		tigMemberInt(tig::action) = tig::actPlayerMeleeAttack;
	}
	else if (action ==  tig::actPlayerMeleeAttack) {
		resolving = updateLunge(dT);
	}

	//TO DO: can do post actions here
	if (!resolving)
		tigMemberInt(tig::action) = tig::actNone;

	return resolving;
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
}


void CPlayerObject::receiveDamage(CGameHexObj& attacker, int damage) {
	THexDir attackDir = neighbourDirection(hexPosition, attacker.hexPosition);

	if (tigMemberInt(tig::equippedShield) == 0) {
		callTig(tig::onReceiveDamage, attacker, damage);
		return;
	}
	int shieldNo = (attackDir - facing) % 6;
	if (shieldNo < 0)
		shieldNo += 6;

	if (shields[shieldNo] > 0) {
		liveLog << "\nYoue shield blocks the blow!";
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

	THexDraw drawData;
	for (int face = 0; face < 6; face++) {
		if (shields[face] == 0)
			continue;
		float rotation = face * -60;
		glm::mat4 rot = glm::rotate(worldMatrix, glm::radians(rotation), glm::vec3(0, 0, 1));
		drawData = { &rot, &colour, shieldBuf };
		hexRendr->drawLines(drawData);
	}
}


void CPlayerObject::setShield(THexDir shieldDir) {
	if (tigMemberInt(tig::equippedShield) == 0) {
		return;
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
}


