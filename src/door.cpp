#include "door.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "ihexworld.h"

#include "hex/hex.h"

CDoor::CDoor() {
	status = doorClosed;
	anim = 0;
	doorSpeed = 4.0f;
}


void CDoor::setLineModel(const std::string& name) {

	CHexObject::setLineModel(name);
	doorNode = lineModel.getNode("Door");
}

void CDoor::frameUpdate(float dT) {
	//if opening or closing, carry on and return

	if (status == doorOpening) {
		float shift = dT * doorSpeed;
		anim += shift;
		if (anim > 1.0f) {
			anim = 1.0f;
			status = doorOpen;
		}
		doorNode->matrix = glm::translate(doorNode->matrix, glm::vec3(0, shift, 0));
		return;
	}

	if (status == doorClosing) {
		float shift = dT * doorSpeed;
		anim += shift;
		if (anim > 1.0f) {
			anim = 1.0f;
			status = doorClosed;
		}
		doorNode->matrix = glm::translate(doorNode->matrix, glm::vec3(0, -shift, 0));
		return;
	}



	//otherwise, check if we need to start


	//check facing and opposite hexes for presence
	CHex facingHex = getNeighbour(hexPosition, facing);
	CHex oppositeHex = getNeighbour(hexPosition,opposite(facing));
	if (hexWorld->getEntityAt(facingHex) || hexWorld->getEntityAt(oppositeHex)) {
		
		if (/*status != doorOpening && */status == doorClosed) {
			blocks = false;
			anim = 0;
			status = doorOpening;
		}
	}
	else { //no one there. Close?
		if (status == doorOpen && hexWorld->getBlockingEntityAt(hexPosition) == NULL) {
			blocks = true;
			anim = 0;
			status = doorClosing;
		}


	}



	return ;
}
