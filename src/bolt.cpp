#include "bolt.h"

#include "gamehextObj.h"
#include "IHexWorld.h"
CBolt::CBolt() {
	moveSpeed = 30.0f;
}

/** Initialise travel to the destination hex. */
void CBolt::fireAt(CHex& target) {
	worldSpaceDestination = cubeToWorldSpace(target);

	moveVector = worldSpaceDestination - worldPos;
	moveVector.z = 0;
	moveVector = glm::normalize(moveVector);
	float dot = glm::dot(moveVector, glm::vec3(1, 0, 0));
	rotation = glm::acos(glm::dot(moveVector, glm::vec3(1, 0, 0)));
	if (moveVector.y > 0)
		rotation = 2*M_PI - rotation;
	buildWorldMatrix();
}

bool CBolt::update(float dT) {
	float resolving = updateMove(dT);

	if (!resolving) {
		CGameHexObj* targetObj = hexWorld->getEntityAt(worldSpaceToHex(worldPos));
		if (targetObj) {
			targetObj->receiveDamage(*hexWorld->getPlayerObj(), 1);
		}

		//hexWorld->removeGridObj(*this);

	}


	return resolving;
}



