#include "gameGui.h"

#include <map>

#include "..\\3DEngine\src\UI\GUIroot.h"
#include "GameTextWin.h"

std::map<std::string, CGameTextWin*> windows;

void gWin::createWin(const std::string& winName, int x, int y, int w, int h) {
	CGameTextWin* panel = new CGameTextWin();
	panel->resize(w, h);
	panel->setLocalPos(x, y);

	panel->anchorRight = 10;
	//panel->setTheme("smallNormal");

	pGUIroot->add(panel);
	windows[winName] = panel;
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
}

