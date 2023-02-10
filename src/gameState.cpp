#include "gameState.h"

#include "robot.h"

CGameState game;


void CGameState::setMap(CMap* map) {
	this->map = map;
}



void CGameState::deleteEntity(CEntity& entity) {
	map->deleteEntity(entity);
}

void CGameState::killEntity(CEntity& entity) {
	entity.live = false; 
	entity.deleteMe = true;
	map->entitiesToKill = true;
}

void CGameState::update(float dT) {

	map->tidyEntityLists();

}

void CGameState::togglePause() {
	paused = !paused;
}


void CGameState::toggleUImode(bool onOff) {
	uiMode = onOff;
}
