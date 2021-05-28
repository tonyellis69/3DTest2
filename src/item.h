#pragma once

#include "entity.h"

#include <string>


class CItem : public CEntity {
public:
	CItem();

	std::string itemType; //temp!
	std::string description; //temp!

};

