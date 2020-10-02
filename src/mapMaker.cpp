#include "mapMaker.h"

#include "hexItem.h"
#include "door.h"
#include "robot.h"

void CMapMaker::attachVM(Ivm* pVM) {
	vm = pVM;

}

CGameHexArray* CMapMaker::makeMap(ITigObj* mapObj) {
	this->mapObj = mapObj;

	CGameHexArray* map = createMap();

	//add robots
	ITigObj* pRobot = vm->getObject(tig::botA);;

	map->updateBlocking();

	//for (int x = 0; x < 1; x++) {
	//	CRobot* robotM = new CRobot();
	//	robotM->setLineModel("robot");
	//	//map->addActor(robotM, map->findRandomHex(true));
	//	map->addActor(robotM, CHex(-2,-4,6));
	//	robotM->setTigObj(pRobot);
	//	robotM->setGoalWander();
	//}

	//add items & door
	tempPopulateMap();


	return map;
}

void CMapMaker::attachMapObject(ITigObj* mapObj) {
	this->mapObj = mapObj;
}
static unsigned int rando;
/** Return a map on a hex array, built according to the specifications found in
	the previously assigned Tig map object .*/
CGameHexArray* CMapMaker::createMap() {

	randEngine.seed(rando++);


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

	hexArray = new CGameHexArray();

	hexArray->init(boundingBox.x, boundingBox.y);


	using param_t = std::uniform_int_distribution<>::param_type;
	param_t x{ 0, hexArray->width - 1 };
	randX.param(x); 
	param_t y{ 0, hexArray->height - 1 };
	randY.param(y);

	glm::i32vec2 tL = margin;
	glm::i32vec2 bR = boundingBox - margin;


		for (int y = 0; y < boundingBox.y; y++) {
			for (int x = 0; x < boundingBox.x; x++) {
				if (x < tL.x || x >= bR.x || y < tL.y || y >= bR.y)
					hexArray->getHexOffset(x, y).content = 2;
				else
					hexArray->getHexOffset(x, y).content = 1;
				hexArray->getHexOffset(x, y).fogged = 1.0f;
			}
		}


		tmpAddBlocks();
		hexArray->updateBlocking();
		tmpAddDesks();

	//} while (!hexArray->isValidPath(CHex(-13, 5, 8), CHex(13, -4, -9)));

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
	} while (!hexArray->isEmpty(randHex));
	return randHex;
}

glm::i32vec2 CMapMaker::randomAdjacentHex(glm::i32vec2& hex) {
	CHex cubePos = hexArray->indexToCube(hex.x,hex.y);
	std::vector<CHex> freeNeighbours;
	for (int h = 0; h < 6; h++) {
		THexDir dir = (THexDir)h;
		CHex neighbour = getNeighbour(cubePos, dir);
		if (hexArray->getHexCube(neighbour).content == 1)
			freeNeighbours.push_back(neighbour);
	}
	if (freeNeighbours.empty())
		return { -1,-1 };

	std::uniform_int_distribution<> randNeighbour(0, freeNeighbours.size() - 1);
	CHex neighbour = freeNeighbours[randNeighbour(randEngine)];

	return hexArray->cubeToIndex(neighbour);
}

bool CMapMaker::randomBlockWalk(glm::i32vec2 hex, int numSteps, int depth) {
	//pick a pretty random random free direction
	CHex cubePos = hexArray->indexToCube(hex.x, hex.y);
	THexDir walkDir = randomFreeDir(cubePos);

	static std::uniform_int_distribution<> recurse(1, 5);
	for (int walk = 0; walk < numSteps;) {
		if (recurse(randEngine) == 1 && depth > 0)
			randomBlockWalk(hex, numSteps-1, depth-1);
		else
			hexArray->getHexOffset(hex.x, hex.y).content = 2;

		//pick a weighted random free direction
		//get the hex at that direction
		
		walkDir = semiRandomFreeDir(walkDir, cubePos);
		if (walkDir == hexNone)
			return true;

		hex = hexArray->cubeToIndex( getNeighbour(cubePos, walkDir) );
		cubePos = hexArray->indexToCube(hex.x, hex.y);
		walk++;
	}
	return true;
}

THexDir CMapMaker::semiRandomFreeDir(THexDir lastDir, CHex& cubePos) {
	//can we use the same direction as last time?
	std::uniform_int_distribution<> noVary(0, 1);

	CHex neighbour = getNeighbour(cubePos, lastDir);
	if (hexArray->getHexCube(neighbour).content == 1 && noVary(randEngine) == 1)
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
		if (hexArray->getHexCube(neighbour).content == 1)
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
		CHex deskPos = hexArray->indexToCube(randomFreeHex());
		CGameHexObj* desk = tmpCreateDesk();
		//desk->setPosition(deskPos);
		hexArray->add(desk, deskPos);
	}

}

CGameHexObj* CMapMaker::tmpCreateDesk() {
	CGameHexObj* desk = new CGameHexObj();
	//desk->setMap(&hexArray);
	desk->setLineModel("desk");
	desk->setTigObj(vm->getObject(tig::desk));
	hexArray->entities.push_back(desk);
	return desk;
}

/** Fill the map with its permanent entities. */
void CMapMaker::tempPopulateMap() {

	CHexItem* wrench;
	CHexItem* shield;
	CHexItem* blaster;
	CDoor* door;

	wrench = new CHexItem();
	wrench->setLineModel("test");
	hexArray->add(wrench, CHex(-7, 0, 7));
	wrench->setTigObj(vm->getObject(tig::monkeyWrench));
	hexArray->entities.push_back(wrench);

	shield = new CHexItem();
	shield->setLineModel("test");
	hexArray->add(shield, CHex(-5, -2, 7));
	shield->setTigObj(vm->getObject(tig::shield));
	hexArray->entities.push_back(shield);

	blaster = new CHexItem();
	blaster->setLineModel("test");
	hexArray->add(blaster, CHex(-6, -1, 7));
	blaster->setTigObj(vm->getObject(tig::blaster));
	hexArray->entities.push_back(blaster);

	door = new CDoor();
	door->setLineModel("door");
	hexArray->add(door, CHex(13, -4, -9));
	door->setTigObj(vm->getObject(tig::CDoor));
	door->setZheight(0.01f); //TO DO: sort this!!!!!
	hexArray->entities.push_back(door);
}