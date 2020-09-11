#include "gameState.h"


CGameState world;

void CGameState::setTurnPhase(TTurnPhase phase) {
	turnPhase = phase;
}

TTurnPhase CGameState::getTurnPhase() {
	return turnPhase;
}


void CGameState::onNotify(COnCursorNewHex& msg) {
	cursorPos = msg.newHex;
}
