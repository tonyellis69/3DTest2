#pragma once

#include "hex/hexArray.h"

#include "gamehextObj.h"

#include "IGameHexArray.h"

/** Extend the hexArray to check for game entities when
	pathfinding. */
class CGameHexArray : public CHexArray, public IGameHexArray {
public:
	void setEntityList(TEntities* pEntities);
	bool entityBlockCheck(CHex& hex);
	bool isEmpty(glm::i32vec2& hex);
	CHex findLineEnd(CHex& start, CHex& target);

	TEntities* entities; ///<To check for collision against.

};