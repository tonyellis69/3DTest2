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
	virtual void onRichTextMouseOver(const std::string& msg) {}
	virtual void onMouseOff() {}
	virtual void onRichTextClick(const std::string& msg) {}
	virtual void onMsg(const std::string& msg) {}

	int getEntityNo(const std::string& str);

	CGameTextWin* pWin;
};


using TWinPlugin = std::shared_ptr<CSmartWin>;