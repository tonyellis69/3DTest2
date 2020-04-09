#pragma once

#include <random>

#include "ITigObj.h"
#include "Ivm.h"

#include "gameHexArray.h"

/** A class for creating maps of individual rooms or areas. */
class CMapMaker {
public:

	void attachVM(Ivm* pVM);
	void attachMapObject(ITigObj* mapObj);
	CGameHexArray createMap();

	TEntities* entities;
private:
	void tmpAddBlocks();

	glm::i32vec2 randomFreeHex();

	glm::i32vec2 randomAdjacentHex(glm::i32vec2& hex);

	bool randomBlockWalk(glm::i32vec2 hex, int numSteps, int depth);

	THexDir semiRandomFreeDir(THexDir lastDir, CHex& cubePo);

	THexDir randomFreeDir(CHex& cubePos);

	void tmpAddDesks();

	CGameHexObj* tmpCreateDesk();


	ITigObj* mapObj; ///<The Tig template for the map we're creating.
	Ivm* vm; ///<Interface to the Tig VM.
	CGameHexArray hexArray;


	std::mt19937 randEngine;
	std::uniform_int_distribution<> randX;
	std::uniform_int_distribution<> randY;
};