#pragma once

#include "smartWin.h"

#include "listen/listen.h"

class CInventoryWin : public CSmartWin {
public:
	CInventoryWin() {
		lis::subscribe(this);

	}
	virtual void update(float dT);
	virtual void onRichTextMouseOver(const std::string& msg);
	void onEvent(CEvent& e);

	std::string currentItem;

	int richTextId;

};