#include "healthC.h"

#include "entity.h"

void CHealthC::onAdd() {
	thisEntity->healthC = this;
}


void CHealthC::onRemove() {
	thisEntity->healthC = nullptr;
}