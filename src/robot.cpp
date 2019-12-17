#include "robot.h"


void CRobot::chooseTurnAction() {
	//default choice: chase player!
	findTravelPath(callbackObj->getPlayerPositionCB());
	newMove();
}

bool CRobot::update(float dT) {
	bool result = CHexObject::update(dT);

	if (!moving) {
		//are we adjacent to player?
		//then turn to face him before we end our turn
		for (int neighbourDir = hexEast; neighbourDir <= hexNE; neighbourDir++) {
			CHex neighbour = getNeighbour(hexPosition, (THexDir)neighbourDir);
			CHexObject* entity = callbackObj->getEntityAtCB(neighbour);
			if (entity && !entity->isRobot) {
				if (facing != neighbourDir) {
					

					destinationDirection = (THexDir)neighbourDir;
					float rotationDir = shortestRotation(facing, destinationDirection);
					moving = true;
					turning = true;
					rotationalVelocity = (rotationDir > 0) - (rotationDir < 0);
					destinationAngle = dirToAngle(destinationDirection);
					return false;

				}

			}
				
		}
	}

	return result;
}


