#include "gameTextWin.h"

CGameTextWin::CGameTextWin() {
	//artbitrary size
	resize(300, 200);
	setLocalPos(500, 500);

	totalBorderSize = { style::gameWinCtrBorder.x * 2, style::gameWinCtrBorder.y * 2 };

	richText = new CGUIrichText(style::gameWinCtrBorder.x, style::gameWinCtrBorder.y,
		500, 200);
	richText->resizeMax = { 400, 500 };
	richText->setTextTheme("gameTheme");
	richText->setHotTextVM(pVM);
	Add(richText);
}

void CGameTextWin::addText(const std::string& text) {
	richText->appendMarkedUpText(text);
}

void CGameTextWin::setTheme(const std::string& themeName) {
	richText->setTextTheme(themeName);
}

void CGameTextWin::setStyle(const std::string& styleName) {
	richText->setTextStyle(styleName);
}

void CGameTextWin::clearText() {
	richText->clear();
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

/** Extends normal resize to ensure we resize richText ctrl. */
void CGameTextWin::resize(int w, int h) {
	CGameWin::resize(w, h);
	if (richText == NULL)
		return;
	richText->setLocalPos(style::gameWinCtrBorder.x, style::gameWinCtrBorder.y);
	richText->resize(w - 2 * style::gameWinCtrBorder.x, h - 2 * style::gameWinCtrBorder.y);
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

void CGameTextWin::positionOffset() {
	hFormat = hRight;
	vFormat = vTop;
}

void CGameTextWin::onNotify(CMouseExitHex& msg) {
	setVisible(false);

}