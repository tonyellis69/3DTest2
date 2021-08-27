#pragma once 

#include <string>

#include "messaging/msg.h"


#include "hex/hex.h"



class CSysMsg : public CMsg {
public:
	CSysMsg(bool b) : isOn(b) {}
	bool isOn;
};


enum TPopup { defencePopup, statusPopup, powerQ, combatLog };

class CSendText : public CMsg {
public:
	CSendText(TPopup popType, const std::string& txt, bool c = false) : popupType(popType),
	text(txt), clear(c) {}

	std::string text;
	TPopup popupType;
	bool clear = false;
};


class CTurnBegin : public CMsg {
public:
	CTurnBegin() {}
};



class CEntity;
class CActorMovedHex : public CMsg {
public:
	CActorMovedHex(CHex& h, CEntity* a) :
		newHex(h), actor(a) {}

	CHex newHex;
	CEntity* actor;
};



