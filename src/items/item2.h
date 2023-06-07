#pragma once

#include "../entity/component.h"

//CEntityCmp version of item baseclass

class CItemCmp : public CDerivedC<CItemCmp> {
public:
	CItemCmp(CEntity* parent) : CDerivedC(parent) {}
	void onAdd();
	void onRemove();
	void setOwner(CEntity* owner);
	void onSpawn();

	CEntity* itemOwner = nullptr;
	int ownerId;
};