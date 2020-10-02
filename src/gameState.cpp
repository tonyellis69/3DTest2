#include "gameState.h"


CGameState world;

void CGameState::setTurnPhase(TTurnPhase phase) {
	turnPhase = phase;
}

void CGameState::setMap(CGameHexArray* map) {
	this->map = map;
}

TTurnPhase CGameState::getTurnPhase() {
	return turnPhase;
}


void CGameState::onNotify(COnCursorNewHex& msg) {
	cursorPos = msg.newHex;
}


/** Returns true if an entity can't move to this adjacent hex.*/
bool CGameState::isBlocked(CHex& pos, CHex& dest) {
	return map->fromToBlocked(pos, dest);
}
