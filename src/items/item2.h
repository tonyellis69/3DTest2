#pragma once

#include "../entity/component.h"

//CEntityCmp version of item baseclass

class CItemCmp : public CEntityCmp {
public:
	CItemCmp(CEntity* parent) : CEntityCmp(parent) {}
	void setOwner(CEntity* owner);
	void onSpawn();

	CEntity* itemOwner = nullptr;
	int ownerId;
};