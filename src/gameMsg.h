#pragma once 

#include <string>

#include "messaging/msg.h"

#include "hex/hex.h"

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