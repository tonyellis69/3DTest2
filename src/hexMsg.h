#pragma once

#include "messaging/msg.h" 
#include "hex/hex.h"

class CMouseNewHex : public CMsg {
public:

	CHex newHex;
};