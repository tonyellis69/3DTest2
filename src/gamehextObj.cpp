#include "gamehextObj.h"

#include "tigConst.h"

CGameHexObj::CGameHexObj() {
	lungeSpeed = 3.0f;
	meleeDamage = 1;
	mBlocks = blocksAll;
	deleteMe = false;
}



bool CGameHexObj::isNeighbour(CGameHexObj& obj) {
	return ::isNeighbour(hexPosition,obj.hexPosition);
}

bool CGameHexObj::isNeighbour(CHex& hex) {
	return ::isNeighbour(hexPosition, hex);
}

int CGameHexObj::getChosenAction() {
	return tigMemberInt(tig::action);
}

std::string CGameHexObj::getName() {
	return tigMemberString(tig::name);
}


/** Returns full blocking details as bit flags. */
unsigned int CGameHexObj::blocks() {
	return mBlocks;
}

/** Returns true if this entity blocks entry from the given direction. */
bool CGameHexObj::blocks(THexDir direction) {
	unsigned int dirBit = 1 << direction;
	return blocks() & dirBit;
}

void CGameHexObj::receiveDamage(CGameHexObj& attacker, int damage) {
	;// callTig("onReceiveDamage", attacker, damage);
}

