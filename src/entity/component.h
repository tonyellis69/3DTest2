#pragma once


class CEntity;
class CEntityCmp {
public:
	CEntityCmp() = default;
	CEntityCmp(CEntity* parent) : thisEntity(parent) {}
	virtual void update(float dT) {}
	virtual void onSpawn() {}


	CEntity* thisEntity;

};