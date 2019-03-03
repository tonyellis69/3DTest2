#pragma once

//#include "3DtestApp.h"
#include "vm.h"
#include "UI\GUIrichText.h"
//#include "UI\GUIpopMenu.h"
#include "UI\GUIrichTextPanel.h"
#include "localHotList.h"


 
struct TObjWindow {
	CGUIrichTextPanel* win;
	int objId;
};

class C3DtestApp;

/** An interface between the player and a game world running on the 
	Tig virtual machine. */
class CWorldUI {
public:
	CWorldUI();
	void setVM(CTigVM* vm);
	void setGameApp(C3DtestApp* app);
	void init();
	void start();
	void appendText(std::string& text, int window);
	void mainWindowClick(unsigned int hotId, glm::i32vec2 mousePos);
	void inventoryClick(unsigned int hotId, glm::i32vec2 mousePos);

	void handleRoomChange(int direction);
	void openWindow(int winId);
	void openMenuWindow(int winId);
	void openObjWindow(int objId);
	void purge(unsigned int id);
	void clearWindow(int window);

	void showPopupMenu(CGUIrichTextPanel* popControl, const glm::i32vec2& mousePos);
	//void menuClick(const int choice, int objId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp);
	void menuClick(unsigned int hotId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp);


	//void objWindowClick(const int msgId, int objId, glm::i32vec2 & mousePos, CGUIrichTextPanel * popUp);
	void objWindowClick(unsigned int hotId, glm::i32vec2 mousePos, CGUIrichTextPanel * popUp);

	void closeObjWindow(CGUIrichTextPanel * popUp);

	
	void setHottextColour(const glm::vec4& colour);
	void setHottextSelectColour(const glm::vec4& colour);

	void vmMessage(int p1, int p2);

	void createTextWindow();

	void createInventoryWindow();

	CGUIrichTextPanel * spawnPopText();

	void createTextStyles();

	void hide(bool onOff);
	
	void reset();

	//unsigned int textWindowID;
	//CGUIrichText* textWindow;
	CGUIrichTextPanel* mainTextPanel;
	unsigned int mainTextWindowID;

	//CGUIrichText* invWindow;
	//unsigned int invWindowID;
	CGUIrichTextPanel* invPanel;
	unsigned int invPanelID;

	CGUIrichTextPanel* popupPanel;
	unsigned int popupPanelID;
	unsigned int popupTextID;

	TtextTheme normalTheme; ///<Groups normal styles for rich text.
	TtextTheme smallNormalTheme; ///<Groups normal styles for rich text, smaller font size.

private:
	CTigVM* pVM;
//	CGUIrichText* pInvWindow;
//	CGUIrichText* currentTextWindow;
	CGUIrichTextPanel* pMenuWindow;

	C3DtestApp* pApp;

	CFont* popHeaderFont;
	CFont* popBodyFont;

	int playerId; ///<Id of the player object.

	glm::i32vec2 currentMousePos;
	glm::i32vec2 lastMenuCorner;

	

	glm::vec4 darkGray;
	glm::vec4 white;
	glm::vec4 hottextColour;
	glm::vec4 hottextSelectedColour;

	std::vector<TObjWindow> objWindows;


	CFont mainFont;
	CFont mainFontBold;

};


const int popMenuId = 5000;
const int popObjWinId = 5001;

//tig script constants TO DO: can read these from the bytecode
const int mainWin = 0;
const int invWin = 1;
const int menuWin = 2;
const int msgRoomChange = 5000;
