#include "phys.h"

#include "listen/listen.h"
#include "../physEvent.h"

void CPhys::onAdd() {
	CPhysicsEvent e;
	e.entity = thisEntity;
	e.action = physAdd;
	lis::event(e);

	thisEntity->phys = this;
}

void CPhys::onRemove() {
	CPhysicsEvent e;
	e.entity = thisEntity;
	e.action = physRemove;
	lis::event<CPhysicsEvent>(e);

	thisEntity->phys = nullptr;
}
