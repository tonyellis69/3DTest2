#include "mapMaker.h"

void CMapMaker::attachVM(Ivm* pVM) {
	vm = pVM;

}

void CMapMaker::attachMapObject(ITigObj* mapObj) {
	this->mapObj = mapObj;
}

CHexArray CMapMaker::createMap() {
	//read map object

	//create hex array based on readings


	int mapSize = mapObj->getMemberInt("size");

	glm::i32vec2 arraySize;
	if (mapSize == vm->getConst("large")) {
		arraySize = { 30,30 };
	}
	else if (mapSize == vm->getConst("medium")) {
		arraySize = { 20,20 };
	}
	else if (mapSize == vm->getConst("small")) {
		arraySize = { 10,10 };
	}


	glm::i32vec2 margin(1);
	glm::i32vec2 boundingBox = arraySize + margin * 2;

	CHexArray hexArray;
	hexArray.init(boundingBox.x, boundingBox.y);

	glm::i32vec2 tL = margin;
	glm::i32vec2 bR = boundingBox - margin;

	for (int y = 0; y < boundingBox.y; y++) {
		for (int x = 0; x < boundingBox.x; x++) {
			if (x < tL.x || x >= bR.x || y < tL.y || y >= bR.y)
				hexArray.getHexOffset(x, y).content = 2;
			else
				hexArray.getHexOffset(x, y).content = 1;
		}
	}

	return hexArray;



	//create a few walls
	/* for (int s = 0; s < 8; s++) {
		hexArray.getHexOffset(7 + s, 7).content = 2;
		hexArray.getHexOffset(14, 7 + s).content = 2;

		hexArray.getHexOffset(18 + s, 14).content = 2;
		hexArray.getHexOffset(25, 14 + s).content = 2;
	} */
}
