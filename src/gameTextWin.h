#pragma once

#include <string>

#include "gameWin.h"
#include "UI/GUIrichText.h"

/** A window consisting of a rich text control. */
class CGameTextWin : public CGameWin {
public:
	CGameTextWin();
	void addText(const std::string& text);
	void resizeToFit();
	void message(CGUIbase* sender, CMessage& msg);

	void positionAtMousePointer();

	inline static Ivm* pVM; ///<Virtual machine to send hot text calls to.

private:
	CGUIrichText* richText;
	glm::i32vec2 totalBorderSize; ///<Total space taken up by border furniture.
};