#pragma once

#include "../entity.h"

/** Items are a distinct class of entity that the player can
	use and typically carry. */
class CItem : public CEntity {
public:
	CItem() { isItem = true; }; //TO DO: temp! Find better way!
	void setParent(CEntity* parent);

	void setModel(CModel& model);

	void draw();

	virtual void drop() {};
	virtual void take(CEntity* taker);

	CEntity* parent = nullptr; ///<Enity wielding, carrying, etc, this item


};