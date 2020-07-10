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
	CSendText(TPopup popType, const std::string& txt) : popupType(popType),
	text(txt) {}

	std::string text;
	TPopup popupType;
};


class CGetPlayerPos : public CMsg {
public:
	CHex position;
};

class CGetTravelPath : public CMsg {
public:
	CGetTravelPath(CHex& A, CHex& B) : start(A), end(B) {}
	CHex start;
	CHex end;
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
	CDiceRoll(int d) : die(d) {}

	int die;
	int result;
};


