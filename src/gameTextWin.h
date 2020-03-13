#pragma once

#include <string>

#include "gameWin.h"
#include "UI/GUIrichText.h"

/** A window consisting of a rich text control. */
class CGameTextWin : public CGameWin {
public:
	CGameTextWin();
	void addText(const std::string& text);


private:
	CGUIrichText* richText;
};