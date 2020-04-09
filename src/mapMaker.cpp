#include "mapMaker.h"

void CMapMaker::attachVM(Ivm* pVM) {
	vm = pVM;

}

void CMapMaker::attachMapObject(ITigObj* mapObj) {
	this->mapObj = mapObj;
}

/** Return a map on a hex array, built according to the specifications found in
	the previously assigned Tig map object .*/
CGameHexArray CMapMaker::createMap() {
	int mapSize = mapObj->tigMemberInt("size");

	glm::i32vec2 arraySize;
	if (mapSize == vm->getConst("largeMap")) {
		arraySize = { 30,30 };
	}
	else if (mapSize == vm->getConst("mediumMap")) {
		arraySize = { 20,20 };
	}
	else if (mapSize == vm->getConst("smallMap")) {
		arraySize = { 10,10 };
	}


	glm::i32vec2 margin(1);
	glm::i32vec2 boundingBox = arraySize + margin * 2;


	hexArray.init(boundingBox.x, boundingBox.y);

	hexArray.setEntityList(entities);

	using param_t = std::uniform_int_distribution<>::param_type;
	param_t x{ 0, hexArray.width - 1 };
	randX.param(x); 
	param_t y{ 0, hexArray.height - 1 };
	randY.param(y);

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

	tmpAddBlocks();

	tmpAddDesks();


	return hexArray;
}


void CMapMaker::tmpAddBlocks() {
	int numBlocks = 20;
	int numSteps = 4;// 5;

	for (int block = 0; block < numBlocks; ) {
		glm::i32vec2 hex = randomFreeHex();
		randomBlockWalk(hex, numSteps, 0);
		block++;	
		
	}

}


glm::i32vec2 CMapMaker::randomFreeHex() {
	glm::i32vec2 randHex;
	do {
		randHex = { randX(randEngine),randY(randEngine) };
	//} while (hexArray.getHexOffset(randHex.x, randHex.y).content != 1);
	} while (!hexArray.isEmpty(randHex));

	return randHex;
}

glm::i32vec2 CMapMaker::randomAdjacentHex(glm::i32vec2& hex) {
	CHex cubePos = hexArray.indexToCube(hex.x,hex.y);
	std::vector<CHex> freeNeighbours;
	for (int h = 0; h < 6; h++) {
		THexDir dir = (THexDir)h;
		CHex neighbour = getNeighbour(cubePos, dir);
		if (hexArray.getHexCube(neighbour).content == 1)
			freeNeighbours.push_back(neighbour);
	}
	if (freeNeighbours.empty())
		return { -1,-1 };

	std::uniform_int_distribution<> randNeighbour(0, freeNeighbours.size() - 1);
	CHex neighbour = freeNeighbours[randNeighbour(randEngine)];

	return hexArray.cubeToIndex(neighbour);
}

bool CMapMaker::randomBlockWalk(glm::i32vec2 hex, int numSteps, int depth) {
	//pick a pretty random random free direction
	CHex cubePos = hexArray.indexToCube(hex.x, hex.y);
	THexDir walkDir = randomFreeDir(cubePos);

	static std::uniform_int_distribution<> recurse(1, 5);
	for (int walk = 0; walk < numSteps;) {
		if (recurse(randEngine) == 1 && depth > 0)
			randomBlockWalk(hex, numSteps-1, depth-1);
		else
			hexArray.getHexOffset(hex.x, hex.y).content = 2;

		//pick a weighted random free direction
		//get the hex at that direction
		
		walkDir = semiRandomFreeDir(walkDir, cubePos);
		if (walkDir == hexNone)
			return true;

		hex = hexArray.cubeToIndex( getNeighbour(cubePos, walkDir) );
		cubePos = hexArray.indexToCube(hex.x, hex.y);
		walk++;
	}
	return true;
}

THexDir CMapMaker::semiRandomFreeDir(THexDir lastDir, CHex& cubePos) {
	//can we use the same direction as last time?
	std::uniform_int_distribution<> noVary(0, 1);

	CHex neighbour = getNeighbour(cubePos, lastDir);
	if (hexArray.getHexCube(neighbour).content == 1 && noVary(randEngine) == 1)
		return lastDir;

	//no? pick random direction
	return randomFreeDir(cubePos);
}


/** Return a random free direction or none. */
THexDir CMapMaker::randomFreeDir(CHex& cubePos) {
	std::vector<THexDir> freeDirections;
	for (int h = 0; h < 6; h++) {
		THexDir dir = (THexDir)h;
		CHex neighbour = getNeighbour(cubePos, dir);
		if (hexArray.getHexCube(neighbour).content == 1)
			freeDirections.push_back(dir);
	}
	if (freeDirections.empty())
		return hexNone;

	std::uniform_int_distribution<> randNeighbour(0, freeDirections.size() - 1);
	return freeDirections[randNeighbour(randEngine)];
}


void CMapMaker::tmpAddDesks() {
	int numDesks = 7;
	for (int d = 0; d < numDesks; d++) {
		CHex deskPos = hexArray.indexToCube(randomFreeHex());
		CGameHexObj* desk = tmpCreateDesk();
		desk->setPosition(deskPos);

	}

}

CGameHexObj* CMapMaker::tmpCreateDesk() {
	CGameHexObj* desk = new CGameHexObj();
	desk->setBuffer("desk");
	desk->setTigObj(vm->getObject(tig::desk));
	entities->push_back(desk);
	return desk;
}