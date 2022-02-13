#pragma once

#include <memory>
#include <string>

/** Base class for a plug-in for game windows to make them smart.*/
class CGameTextWin;
class CSmartWin {
public:
	void setWindow(CGameTextWin* win) {
		pWin = win;
	}
	virtual void update(float dT) {}
	virtual void clearText();
	virtual void addText(const std::string& text);
	virtual void hideWin();
	virtual void showWin();
	virtual void centreOnMouse();
	virtual void removeItem(int itemNo) {}
	virtual void refresh() {}

	CGameTextWin* pWin;
};


using TWinPlugin = std::shared_ptr<CSmartWin>;