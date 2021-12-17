#pragma once

#include "smartWin.h"

class CInventoryWin : public CSmartWin {
public:
	virtual void update(float dT);
	virtual void onRichTextMouseOver(const std::string& msg);
	//virtual void onRichTextClick(const std::string& msg) {}
	void onMouseOff();

	std::string currentItem;
};