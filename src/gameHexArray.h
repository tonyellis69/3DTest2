#pragma once

#include "hex/hexArray.h"

#include "gamehextObj.h"

/** Extend the hexArray to check for game entities when
	pathfinding. */
class CGameHexArray : public CHexArray {
public:
	void setEntityList(TEntities* pEntities);
	bool entityCheck(CHex& hex);


	TEntities* entities; ///<To check for collision against.

};