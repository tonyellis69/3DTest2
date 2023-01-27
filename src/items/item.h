#pragma once

#include "..\entity\entity.h"

/** Items are a distinct class of entity that the player can
	use and typically carry. */
class CItem : public CEntity {
public:
	CItem() { isItem = true; }; //TO DO: temp! Find better way!
	void setParent(CEntity* parent);


	virtual void drop();
	virtual void take(CEntity* taker);

	void examine();
	virtual std::string getMenuTextInv();
	virtual std::string getMenuTextNear();

	std::string getShortDesc() {
		return std::string("tmp");
	}
	//FIXME: above only here to fix error

	CEntity* parent = nullptr; ///<Enity wielding, carrying, etc, this item
	bool itemMenuOpen = false;

	bool equippable = false;
};