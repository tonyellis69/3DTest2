#pragma once

#include "messaging/msg.h" 
#include "hex/hex.h"

class COnNewHex : public CMsg {
public:
	CHex newHex;
};

class CMouseExitHex : public CMsg {
public:
	CHex leavingHex;
};