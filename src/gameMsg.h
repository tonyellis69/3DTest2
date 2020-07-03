#pragma once 

#include <string>

#include "messaging/msg.h"


#include "hex/hex.h"


class CSysMsg : public CMsg {
public:
	CSysMsg(bool b) : isOn(b) {}
	bool isOn;
};


enum TPopup { defencePopup, statusPopup };

class CPopupText : public CMsg {
public:
	CPopupText(TPopup popType, const std::string& txt) : popupType(popType),
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

class CShootAt : public CMsg {
public:
	CShootAt(CHex& s, CHex& t) : start(s), target(t) {}
	CHex start;
	CHex target;
};

class CSetPlayerAction : public CMsg {
public:
	CSetPlayerAction(int actId, CHex t = CHex(-1, -1, -1))
		: action(actId), target(t) {}
	int action;
	CHex target;
};

