#pragma once

#include "../entity.h"

/** Items are a distinct class of entity that the player can
	use and typically carry. */
class CItem : public CEntity {
public:

	void setParent(CEntity* parent);

	CEntity* parent; ///<Enity wielding, carrying, etc, this item


};