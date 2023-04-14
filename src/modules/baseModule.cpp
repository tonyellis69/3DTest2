#include "baseModule.h"

#include "../gameState.h"

/** Store the current state of the game's entities. */
void CBaseModule::saveEntityState() {
	entityState.clear();
	for (auto& ent : game.entities) {
		entityState.push_back(std::make_shared<CEntity>(*ent));
	}
}

/** Set the state of the game's entities to what we have stored. */
//void CBaseModule::restoreEntityState() {
//	game.restoreEntities(entityState);
//}
