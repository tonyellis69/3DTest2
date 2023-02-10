#pragma once

enum TPhysEvent {physAdd, physRemove};
class CPhysicsEvent {
public:
	CPhysicsEvent() {}
	CEntity* entity;
	TPhysEvent action;
};