#include "smartWin.h"

#include "..\gameTextWin.h"
#include "UI/gui2.h"

#include "win/win.h"

void CSmartWin::clearText() {
	pWin->clearText();
}

void CSmartWin::addText(const std::string& text) {
	pWin->addText(text);
}

void CSmartWin::hideWin() {
	pWin->setVisible(false);
}

void CSmartWin::showWin() {
	pWin->setVisible(true);
}

void CSmartWin::centreOnMouse() {
	//glm::i32vec2 mousePos = pGUIroot->getLocalMousePos();
	glm::i32vec2 mousePos;
	CWin::getMousePos(mousePos.x, mousePos.y);

	glm::i32vec2 winSizeHalf = pWin->getSize() / 2;

	pWin->anchorRight = NONE;
	glm::i32vec2 pos = mousePos - winSizeHalf;
	if (pos.y < 10)
		pos.y = 10;

	pWin->setLocalPos(pos);
}




