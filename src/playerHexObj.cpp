#include "playerHexObj.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/integer.hpp>

#include "utils/log.h"



CPlayerObject::CPlayerObject() {

}

bool CPlayerObject::update(float dT) {
	CHex playerOldHex = hexPosition;

	bool resolving = false;
	resolving = updateMove(dT);
	int action = getCurrentAction();
	if (action ==  tig::actPlayerTurnToAttack && !turning) {
		moving = false;
		tigMemberInt(tig::action) = tig::actPlayerAttack;
	}

	if (action ==  tig::actPlayerAttack) {
		resolving = updateLunge(dT);
	}

	if (hexPosition != playerOldHex) {
		hexWorld->onPlayerTurnDoneCB();
	}

	//TO DO: can do post actions here
	if (!resolving)
		tigMemberInt(tig::action) = tig::actNone;

	return resolving;
}

void CPlayerObject::beginAttack(CGameHexObj& target) {
	attackTarget = &target;
	tigMemberInt(tig::action) = tig::actPlayerAttack;
	if (initTurnToAdjacent(target.hexPosition))
		tigMemberInt(tig::action) = tig::actPlayerTurnToAttack;

	animCycle = 0;
	moveVector = directionToVec(destinationDirection);
}

void CPlayerObject::receiveDamage(CGameHexObj& attacker, int damage) {
	THexDir attackDir = neighbourDirection(hexPosition, attacker.hexPosition);

	int shieldNo = (attackDir - facing) % 6;
	if (shieldNo < 0)
		shieldNo += 6;

	callTig(tig::onReceiveDamage, attacker, damage);

	if (shields[shieldNo] > 0) {
		liveLog << "\nYoue shield blocks the blow!";
		return;
	}

}

void CPlayerObject::draw() {
	//draw self
	CHexObject::draw();

	//draw shield:
	int action = tigMemberInt(tig::action);
	if (action ==  tig::actPlayerAttack)
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
	int shieldNo = (shieldDir - facing) % 6;
	if (shieldNo < 0)
		shieldNo += 6;

	shields[shieldNo] = !shields[shieldNo];
}


void CPlayerObject::takeItem(CGameHexObj& item) {
	hexWorld->playerTake(item);
	callTig(tig::onTake, item);
}

void CPlayerObject::showInventory() {
	callTig(tig::onInventory);
}

void CPlayerObject::dropItem(int itemNo) {
	
	ITigObj* item = callTigObj(tig::onDrop, itemNo);
	hexWorld->playerDrop((CGameHexObj*)item->getCppObj());
}


