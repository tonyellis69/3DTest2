#pragma once


class CEntity;
class CEntityCmp {
public:
	CEntityCmp() = default;
	CEntityCmp(CEntity* parent) : parentEntity(parent) {}
	virtual void update(float dT) {}


	CEntity* parentEntity;

};