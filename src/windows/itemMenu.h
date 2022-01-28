#pragma once

#include "smartWin.h"
#include "listen/listen.h"
#include "UI/guiBase2.h"

class CItemMenu : public CSmartWin {
public:
	CItemMenu() {
		lis::subscribe(this);
	}
	void onEvent(CEvent& e);
	void startTimeout();
	void showItemChoices(const std::string& hotTxt);
	void positionLeftOf(CguiBase* spawner);
	void update(float dT);
	void onRichTextClick(const std::string& msg);



private:
	float timer = 0;

	int currentEntityNo = 0;
};