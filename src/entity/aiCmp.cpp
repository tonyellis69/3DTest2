#include "aiCmp.h"

#include "entity.h"

void CAiCmp::onAdd() {
	thisEntity->ai = this;
}

void CAiCmp::onRemove() {
	thisEntity->ai = nullptr;
}