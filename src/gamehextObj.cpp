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
	if (reduceHitPoints(damage)) {
		std::string damageLog = "\n" + getName();
		damageLog += " hit by " + attacker.getName();
		damageLog += " for " + std::to_string(damage) + " damage.\n";

		CSendText msg(combatLog, damageLog);
		send(msg);
	}
}

/** Returns false if this was fatal. */
bool CGameHexObj::reduceHitPoints(int damage) {
	tmpHP -= damage;
	if (tmpHP <= 0) {
		deathRoutine();
		return false;
	}
	return true;
}
