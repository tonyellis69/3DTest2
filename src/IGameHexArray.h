#pragma once 

#include "hex/hex.h"

class IGameHexArray {
public:
	virtual CHex findLineEnd(CHex& start, CHex& target) = 0;


};