#pragma once

#include "entity/entity.h"

enum TPhysEvent {physAdd, physRemove};
class CPhysicsEvent {
public:
	CPhysicsEvent() {}
	CEntity* entity;
	TPhysEvent action;
};