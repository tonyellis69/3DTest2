#pragma once

#include "gamehextObj.h"

#include <string>


class CItem : public CGameHexObj {
public:
	CItem();

	std::string itemType; //temp!
	std::string description; //temp!

};

