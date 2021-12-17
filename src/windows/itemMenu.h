#pragma once

#include "smartWin.h"


class CItemMenu : public CSmartWin {
public:
	void update(float dT);
	//virtual void onRichTextMouseOver(const std::string& msg) {}
	void onRichTextClick(const std::string& msg);
	void onMsg(const std::string& msg);


private:
	float timer = 0;

	int currentEntityNo = 0;
};