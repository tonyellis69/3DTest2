#pragma once

#include <string>

#include "gameWin.h"
#include "UI/GUIrichText.h"

/** A window consisting of a rich text control. */
class CGameTextWin : public CGameWin {
public:
	CGameTextWin();
	void addText(const std::string& text);
	void setTheme(const std::string& themeName);
	void setStyle(const std::string& styleName);
	void clearText();
	bool isEmpty();
	void resizeToFit();
	void message(CGUIbase* sender, CMessage& msg);
	void resize(int w, int h);

	void positionAtMousePointer();
	void positionOffset();

	void onNotify(CMouseExitHex& msg);


	inline static Ivm* pVM; ///<Virtual machine to send hot text calls to.

private:
	CGUIrichText* richText = NULL;
	glm::i32vec2 totalBorderSize; ///<Total space taken up by border furniture.
};