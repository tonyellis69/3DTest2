#include "playerHexObj.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/integer.hpp>

#include "utils/log.h"

CPlayerObject::CPlayerObject() {
	hitPoints = 6;
	
}

bool CPlayerObject::update(float dT) {
	CHex playerOldHex = hexPosition;

	bool resolving = false;
	resolving = updateMove(dT);

	if (action == actPlayerTurnToAttack && !turning) {
		moving = false;
		action = actPlayerAttack;
	}

	if (action == actPlayerAttack) {
		resolving = updateLunge(dT);
	}

	if (hexPosition != playerOldHex) {
		hexWorld->onPlayerTurnDoneCB();
	}

	//TO DO: can do post actions here
	if (!resolving)
		action = actNone;

	return resolving;
}

void CPlayerObject::beginAttack(CHexObject& target) {
	attackTarget = &target;
	action = actPlayerAttack;
	if (initTurnToAdjacent(target.hexPosition))
		action = actPlayerTurnToAttack;

	animCycle = 0;
	moveVector = directionToVec(destinationDirection);
	liveLog << "\nYou hit the robot!";
}

void CPlayerObject::receiveDamage(CHexObject& attacker, int damage) {
	THexDir attackDir = neighbourDirection(hexPosition, attacker.hexPosition);

	int shieldNo = (attackDir - facing) % 6;
	if (shieldNo < 0)
		shieldNo += 6;

	if (shields[shieldNo] > 0) {
		liveLog << "\nYou shield blocks the blow!";
		return;
	}


	hitPoints -= damage;
	if (hitPoints <= 0) {
		liveLog << "\nOh no, you're dead!";

	}
}

void CPlayerObject::draw() {
	//draw self
	CHexObject::draw();

	//draw shield:
	if (action == actPlayerAttack)
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


