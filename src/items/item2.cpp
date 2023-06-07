#include "item2.h"

#include "../gameState.h"

void CItemCmp::onAdd() {
	thisEntity->item = this;
}

void CItemCmp::onRemove() {
	thisEntity->item = nullptr;
}


void CItemCmp::setOwner(CEntity* owner) {
	this->itemOwner = owner;
	ownerId = owner->id;
}

void CItemCmp::onSpawn() {
	itemOwner = game.getEntity(ownerId);
}
