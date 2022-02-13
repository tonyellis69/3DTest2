#include "gameGui.h"

#include <map>

#include "..\\3DEngine\src\UI\GUIroot.h"
#include "GameTextWin.h"

#include "windows/inventoryWin.h"
#include "windows/itemMenu.h"
#include "windows/examWin.h"
#include "windows/nearWin.h"

std::map<std::string, CGameTextWin*> windows;

namespace gWin {
	TWinPlugin pNear;
	TWinPlugin pInv;
	TWinPlugin pItemMenu;
	TWinPlugin pExam;
}

void gWin::initWindows() {

	createWin("near", 10, 10, 200, 300);
	pNear = std::make_shared<CNearWin>();
	setPlugin("near", pNear);

	createWin("inv", 10, 340, 200, 300);
	pInv = std::make_shared<CInventoryWin>();
	setPlugin("inv", pInv);

	createWin("itemMenu", 10, 10, 200, 200);
	pItemMenu = std::make_shared<CItemMenu>();
	setPlugin("itemMenu", pItemMenu);
	pItemMenu->hideWin();

	createWin("exam", 10, 10, 250, 200);
	pExam = std::make_shared<CExamWin>();
	setPlugin("exam", pExam);
	pExam->hideWin();
}

void gWin::createWin(const std::string& winName, int x, int y, int w, int h) {
	CGameTextWin* panel = new CGameTextWin();
	panel->name = winName;
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





void gWin::setPlugin(const std::string& winName, TWinPlugin plugin) {
	windows[winName]->setPlugin(plugin);
}



