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


class CGetPlayerPos : public CMsg {
public:
	CHex position;
};

class CGetTravelPath : public CMsg {
public:
	CGetTravelPath(CHex& A, CHex& B, bool fog = false) : start(A), end(B),
	fogOn(fog) {}
	CHex start;
	CHex end;
	bool fogOn;
	THexList travelPath;
};

class CGetLineEnd : public CMsg {
public:
	CGetLineEnd(CHex& A, CHex& B) : start(A), end(B) {}
	CHex start;
	CHex end;
};

class CTurnBegin : public CMsg {
public:
	CTurnBegin() {}
};

class CDiceRoll : public CMsg {
public:
	CDiceRoll(int d, int d2=0) : die(d), die2(d2) {}

	int die;
	int die2;
	int result;
	int result2 = 0;
};

class CLineOfSight : public CMsg {
public:
	CLineOfSight(CHex& s, CHex& e) : start(s), end(e) {}

	CHex start;
	CHex end;
	bool result = false;
};

class CCalcVisionField : public CMsg {
public:
	CCalcVisionField(CHex& a, THexList& r, bool obv = false) :
		apex(a), perimeterHexes(&r), obsessive(obv) {}

	CHex apex;
	THexList* perimeterHexes;
	THexList visibleHexes;
	bool obsessive;
};

class CRandomHex : public CMsg {
public:
	CRandomHex(bool u = false) : unblocked(u) {}

	CHex hex;
	bool unblocked;
};

class CGameHexObj;
class CActorMovedHex : public CMsg {
public:
	CActorMovedHex(CHex& h, CGameHexObj* a) :
		newHex(h), actor(a) {}

	CHex newHex;
	CGameHexObj* actor;
};



class CPlayerSeen : public CMsg {
public:
	CPlayerSeen(CGameHexObj* v) : viewer(v) {}

	CGameHexObj* viewer;
};

