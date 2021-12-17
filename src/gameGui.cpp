#include "gameGui.h"

#include <map>

#include "..\\3DEngine\src\UI\GUIroot.h"
#include "GameTextWin.h"

std::map<std::string, CGameTextWin*> windows;

void gWin::createWin(const std::string& winName, int x, int y, int w, int h) {
	CGameTextWin* panel = new CGameTextWin();
	panel->resize(w, h);
	panel->setLocalPos(x, y);

	panel->anchorRight = 10; //TO DO should probably scrap this!

	pGUIroot->add(panel);
	windows[winName] = panel;
}

void gWin::update(float dT) {
	for (auto& win : windows)
		win.second->update(dT);
}


void gWin::addText(const std::string& winName, const std::string& text) {
	windows[winName]->addText(text);

}

void gWin::clearText(const std::string& winName) {
	if (windows[winName]->isEmpty())
		return;
	windows[winName]->clearText();
}

void gWin::setDefaultFont(const std::string& winName, const std::string& fontName) {
	windows[winName]->setDefaultFont(fontName);
}

void gWin::setColour(const std::string& winName, glm::vec4& colour) {
	windows[winName]->setColour(colour);
}

void gWin::hideWin(const std::string& winName) {
	windows[winName]->setVisible(false);
}

void gWin::showWin(const std::string& winName) {
	windows[winName]->setVisible(true);
	windows[winName]->timeOut = 0;
}

//void gWin::setHotTextMouseoverHandler(const std::string& winName, std::function<void(const std::string&)> handler) {
//	windows[winName]->setHotTextMouseoverHandler(handler);
//}
//
//void gWin::setHotTextClickHandler(const std::string& winName, std::function<void(const std::string&)> handler) {
//	windows[winName]->setHotTextClickHandler(handler);
//}

void gWin::alignWithMouse(const std::string& winName) {
	//find mouse y pos
	glm::i32vec2 mousePos = pGUIroot->getLocalMousePos();

	CGameTextWin* thisWin = windows[winName];
	glm::i32vec2 winSize = thisWin->getSize();
	thisWin->setPosY(mousePos.y - (winSize.y /2));
}

void gWin::centreOnMouse(const std::string& winName) {
	glm::i32vec2 mousePos = pGUIroot->getLocalMousePos();

	CGameTextWin* thisWin = windows[winName];
	glm::i32vec2 winSizeHalf = thisWin->getSize()/2;

	thisWin->anchorRight = NONE;
	thisWin->setLocalPos(mousePos - winSizeHalf);
}

void gWin::putLeftOf(const std::string& winName, const std::string& parentWin) {
	//find left edge of parent
	glm::i32vec2 pos = windows[parentWin]->getLocalPos();
	CGameTextWin* thisWin = windows[winName];
	glm::i32vec2 size = thisWin->getSize();

	//position back from that
	thisWin->anchorLeft = NONE;
	thisWin->setPosX(pos.x - size.x - 10);
	

}

void gWin::timeOut(const std::string& winName, float time) {
	windows[winName]->timeOut = time;
}

void gWin::hideOnMouseOff(const std::string& winName) {

}

void gWin::setPlugin(const std::string& winName, TWinPlugin plugin) {
	windows[winName]->setPlugin(plugin);
}

void gWin::msg(const std::string& winName, const std::string& msg) {
	windows[winName]->msg(msg);
}

