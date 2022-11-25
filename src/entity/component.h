#pragma once


class CEntity;
class CEntityCmp {
public:
	CEntityCmp(CEntity* parent) : parentEntity(parent) {}
	virtual void update(float dT) {}


	CEntity* parentEntity;

};