#pragma once

//#include "3DtestApp.h"
#include "vm.h"
#include "UI\GUIrichText.h"
#include "UI\GUIpopMenu.h"
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
	CWorldUI() { };
	void setVM(CTigVM* vm);
	void setTextWindow(CGUIrichText* txtWin);
	void setInventoryWindow(CGUIrichText * invWin);
	void setGameApp(C3DtestApp* app);
	void init();
	void start();
	void appendText(std::string& text, int window);
	void mainWindowClick(int msgId, int objId, glm::i32vec2 mousePos);
	void inventoryClick(int msgId, int objId, const glm::i32vec2& mousePos);
	void handleRoomChange(int direction);
	void openWindow(int winId);
	void openMenuWindow(int winId);
	void openObjWindow(int objId);
	void purge(int memberId, int objId);
	void clearWindow(int window);

	void showPopupMenu(CGUIrichTextPanel* popControl, const glm::i32vec2& mousePos);
	void menuClick(const int choice, int objId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp);

	void objWindowClick(const int msgId, int objId, glm::i32vec2 & mousePos, CGUIrichTextPanel * popUp);
	
	


	void setMainBodyStyle( CFont& font, const glm::vec4& colour);
	void setInvBodyStyle(CFont& font, const glm::vec4& colour);
	void setPopBodyStyle(CFont& font, const glm::vec4& colour);
	void setPopHeaderStyle(CFont& font, const glm::vec4& colour);
	void setHottextColour(const glm::vec4& colour);
	void setHottextSelectColour(const glm::vec4& colour);

	void vmMessage(int p1, int p2);

private:
	CTigVM* pVM;
	CGUIrichText* pTextWindow;
	CGUIrichText* pInvWindow;
	CGUIrichText* currentTextWindow;
	CGUIrichTextPanel* pMenuWindow;

	C3DtestApp* pApp;

	CFont* popHeaderFont;
	CFont* popBodyFont;

	int playerId; ///<Id of the player object.

	glm::i32vec2 currentMousePos;
	glm::i32vec2 lastMenuCorner;

	TtextStyle mainBodyStyle; ///<Text style for main window body text.
	TtextStyle invBodyStyle;
	TtextStyle popBodyStyle;
	TtextStyle popHeaderStyle;

	glm::vec4 hottextColour;
	glm::vec4 hottextSelectedColour;

	std::vector<TObjWindow> objWindows;
};


const int popMenuId = 5000;
const int popObjWinId = 5001;

//tig script constants TO DO: can read these from the bytecode
const int mainWin = 0;
const int invWin = 1;
const int menuWin = 2;
const int msgRoomChange = 5000;