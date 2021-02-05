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


void CGameState::destroySprite(CSprite& deadSprite) {
	spriteDeathlist.push_back(&deadSprite);
}

void CGameState::deleteEntity(CGameHexObj& entity) {
	map->deleteEntity(entity);
}

void CGameState::update(float dT) {
	for (auto sprite = spriteDeathlist.begin(); sprite != spriteDeathlist.end(); sprite++) {
		for (auto it = sprites.begin(); it != sprites.end(); it++) {
			if (it->get() == *sprite) {
				sprites.erase(it);
				return;
			}
		}
	}

	spriteDeathlist.clear();

	//TO DO: look into tidying the above into something like the below


	map->tidyEntityLists();
}
