#include "gameState.h"

#include "robot.h"

CGameState world;


void CGameState::setMap(CGameHexArray* map) {
	this->map = map;
}



void CGameState::onNotify(COnCursorNewHex& msg) {
	cursorPos = msg.newHex;
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

/** Do necessary housekeeping for the death of the player. */
void CGameState::onPlayerDeath() {
	for (auto& entity : map->entities) {
		if (entity->isRobot) {
			((CRobot*)entity.get())->setState(robotWander3);


		}

	}
}
