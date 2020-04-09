#include "gameTextWin.h"

CGameTextWin::CGameTextWin() {
	//artbitrary size
	resize(300, 200);
	setLocalPos(500, 500);

	totalBorderSize = { style::richTextInset.x * 2, style::richTextInset.y * 2 };

	richText = new CGUIrichText(style::richTextInset.x, style::richTextInset.y, 
		500, 100);
	richText->resizeMax = { 400, 500 };
	richText->setTextTheme("gameTheme");
	richText->setHotTextVM(pVM);
	Add(richText);

}

void CGameTextWin::addText(const std::string& text) {
	richText->appendMarkedUpText(text);
}

void CGameTextWin::resizeToFit() {
	richText->resizeToFit();

}

void CGameTextWin::message(CGUIbase* sender, CMessage& msg) {
	if (msg.Msg == uiMsgChildResize) {
		glm::i32vec2 newSize = richText->getSize() + totalBorderSize;
		resize(newSize.x, newSize.y);

	}
}

/** Position with top left corner at current mouse position,
adjusting if this places the window outside the screen boundary. */
void CGameTextWin::positionAtMousePointer() {
	glm::i32vec2 pos = mousePos;
	glm::i32vec2 posBR = pos + drawBox.size;

	glm::i32vec2 parentBR = parent->drawBox.pos + parent->drawBox.size;

	if (posBR.y > parentBR.y)
		pos.y -= drawBox.size.y;
	if (posBR.x > parentBR.x)
		pos.x -= drawBox.size.x;

	setLocalPos(pos.x, pos.y);
}
