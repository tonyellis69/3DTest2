#pragma once

#include <random>

#include "ITigObj.h"
#include "Ivm.h"

#include "gameHexArray.h"



 
/** A class for creating maps of individual rooms or areas. */
class CMapMaker {
public:

	void attachVM(Ivm* pVM);
	CGameHexArray* makeMap(ITigObj* mapObj);
	void attachMapObject(ITigObj* mapObj);
	CGameHexArray* createMap();



private:

	bool randomBlockWalk(glm::i32vec2 hex, int numSteps, int depth);

	THexDir semiRandomFreeDir(THexDir lastDir, CHex& cubePo);

	THexDir randomFreeDir(CHex& cubePos);

	void tmpaddDesks();

	TEntity tmpCreateDesk();

	void tempPopulateMap();


	ITigObj* mapObj; ///<The Tig template for the map we're creating.
	Ivm* vm; ///<Interface to the Tig VM.
	CGameHexArray* hexArray;


	std::mt19937 randEngine;
	std::uniform_int_distribution<> randX;
	std::uniform_int_distribution<> randY;
};