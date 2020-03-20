#include "gameTextWin.h"



CGameTextWin::CGameTextWin() {
	//artbitrary size
	resize(300, 200);
	setLocalPos(400, 300);

	richText = new CGUIrichText(style::richTextInset.x, style::richTextInset.y, 
		100, 100);
	richText->setTextTheme("gameTheme");
	Add(richText);

}

void CGameTextWin::addText(const std::string& text) {
	richText->appendMarkedUpText(text);
}
