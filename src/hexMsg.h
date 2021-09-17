#pragma once

#include "messaging/msg.h" 
#include "hex/hex.h"



class CMouseExitHex : public CMsg {
public:
	CHex leavingHex;
};

class CPlayerNewHex : public CMsg {
public:
	CPlayerNewHex(CHex& h) : newHex(h) {};

	CHex newHex;
};

