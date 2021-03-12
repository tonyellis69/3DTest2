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

void CGameState::addSprite(std::shared_ptr<CSprite> sprite) {
	addSpritesList.push_back(sprite);
}


void CGameState::destroySprite(CSprite& deadSprite) {
	//spriteDeathlist.push_back(&deadSprite);
	deadSprite.killMe = true;
}

void CGameState::deleteEntity(CGameHexObj& entity) {
	map->deleteEntity(entity);
}

void CGameState::update(float dT) {
	for (auto& it = sprites.begin(); it != sprites.end(); ) {
		if (it->get()->killMe)
			it = sprites.erase(it);
		else
			it++;
	}

	//TO DO: look into tidying the above into something like the below


	map->tidyEntityLists();

	for (auto& sprite = addSpritesList.begin(); sprite != addSpritesList.end(); sprite++) {
		sprites.push_back(*sprite);
	}
	addSpritesList.clear();

}

void CGameState::togglePause() {
	paused = !paused;
}
