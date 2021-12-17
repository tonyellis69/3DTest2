#include "gameState.h"

#include "robot.h"

CGameState game;


void CGameState::setMap(CMap* map) {
	this->map = map;
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

void CGameState::killEntity(CEntity& entity) {
	entity.live = false; 
	map->entitiesToKill = true;
}

void CGameState::update(float dT) {

	map->tidyEntityLists();

}

void CGameState::togglePause() {
	paused = !paused;
}

/** Do necessary housekeeping for the death of the player. */
void CGameState::onPlayerDeath() {
	for (auto& entity : map->entities) {
		if (entity->isRobot) {
			((CRobot*)entity.get())->setState(robotWander3);


		}

	}
}

void CGameState::toggleUImode(bool onOff) {
	uiMode = onOff;
}
