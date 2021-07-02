#include "gameState.h"


CGameState world;


void CGameState::setMap(CGameHexArray* map) {
	this->map = map;
}



void CGameState::onNotify(COnCursorNewHex& msg) {
	cursorPos = msg.newHex;
}


/** Returns true if an entity can't move to this adjacent hex.*/
bool CGameState::isBlocked(CHex& pos, CHex& dest) {
	return map->fromToBlocked(pos, dest);
}

void CGameState::addSprite(std::shared_ptr<CEntity> sprite) {
	//addSpritesList.push_back(sprite);
	map->entities.push_back(sprite);
}


void CGameState::destroySprite(CEntity& deadSprite) {
	//spriteDeathlist.push_back(&deadSprite);
	//deadSprite.killMe = true;
	deadSprite.deleteMe = true;
}

void CGameState::deleteEntity(CEntity& entity) {
	map->deleteEntity(entity);
}

void CGameState::update(float dT) {

	map->tidyEntityLists();

}

void CGameState::togglePause() {
	paused = !paused;
}
