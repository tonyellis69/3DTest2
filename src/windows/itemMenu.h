#pragma once

#include "smartWin.h"
#include "listen/listen.h"
#include "UI/guiBase2.h"
#include "UI/guiEvent.h"

class CItemMenu : public CSmartWin {
public:
	CItemMenu() {
		lis::subscribe<CGUIevent>(this);
	}
	void onEvent(CGUIevent& e);
	void startTimeout();
	void showItemChoices();
	void positionLeftOf(CguiBase* spawner);
	void alignWithMouse(CguiBase* spawner);
	void update(float dT);
	void onRichTextClick(const std::string& msg);



private:
	float timer = 0;

	int currentItem = 0;
};