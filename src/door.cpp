#include "door.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "hex/hex.h"

//#include "actor.h"
#include "hex/hexArray.h"

CDoor::CDoor() {
	status = doorClosed;
	anim = 0;
	doorSpeed = 4.0f;
	//mBlocks = blocksAll; 
}


void CDoor::setLineModel(const std::string& name) {
	CEntity::setBoundingRadius();
	//doorNode = lineModel.getNode("Door");
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
	
	//CGetActorAt facingMsg(facingHex);
	//send(facingMsg);

	//CGetActorAt opMsg(oppositeHex);
	//send(opMsg);

	//CGetActorAt blockerMsg(hexPosition,(CHexActor*) this);
	//send(blockerMsg);

	//if (facingMsg.actor || opMsg.actor || blockerMsg.actor) {
	//	if (status == doorClosed) {
	//		mBlocks = blocksAsDoor;
	//		anim = 0;
	//		status = doorOpening;
	//	}
	//}
	//else { //no one there. Close?
	//	if (status == doorOpen){
	//		mBlocks = blocksAll;
	//		anim = 0;
	//		status = doorClosing;		
	//	}


	//}



	return ;
}

unsigned int CDoor::blocks() {
	////unsigned char rot = mBlocks;
	//for (int dir = hexEast; dir < facing; dir++) {
	//	rot = (rot << 1) | (rot >> 5);
	//}
	//return rot;
	return NULL;
}