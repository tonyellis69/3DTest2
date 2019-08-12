#pragma once

#include <random>
#include <queue>

//#include "3DtestApp.h"
#include "vmApp.h"
#include "UI\GUIrichText.h"
//#include "UI\GUIpopMenu.h"
#include "UI\GUIrichTextPanel.h"
#include "localHotList.h"


 
struct TObjWindow {
	CGUIrichTextPanel* win;
	int objId;
	float lifeTime;
};

class C3DtestApp;

/** An interface between the player and a game world running on the 
	Tig virtual machine. */
class CWorldUI : public Icallback {
public:
	CWorldUI();
	void setVM(CTigVM* vm);
	void setGameApp(C3DtestApp* app);
	void init();
	void start();
	bool writeText(std::string& text, int window);
	void mainWindowClick(unsigned int hotId, glm::i32vec2 mousePos);
	void mainWindowRightClick(glm::i32vec2 mousePos);
	void inventoryClick(unsigned int hotId, glm::i32vec2 mousePos);

	void playerTurn(unsigned int actionHotId);

	void handleRoomChange(int direction);
	void openWindow(int winId, bool modal);
	void openMenuWindow(int winId);
	void spawnPopupWindow(int objId);
	void purgeMainPanel(unsigned int id);
	bool clearWindow(int window);
	void clearMarkedText(int window);

	void clearWindowHotIds(CGUIrichTextPanel* panel);

	void positionWindow(CGUIrichTextPanel* popControl, const glm::i32vec2& mousePos);
	//void menuClick(const int choice, int objId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp);
	void menuClick(unsigned int hotId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp);

	void deletePopupMenu(CGUIrichTextPanel* popUp);


	void objWindowClick(unsigned int hotId, glm::i32vec2 mousePos, CGUIrichTextPanel * popUp);
	void objWindowRightClick(CGUIrichTextPanel* popUp);

	void closeObjWindow(CGUIrichTextPanel * popUp);

	void deletePopupWindow(int id);

	
	void setHottextColour(const glm::vec4& colour);
	void setHottextSelectColour(const glm::vec4& colour);

	void vmMessage(int p1, int p2);

	void queueMsg(TvmAppMsg& msg);

	void createMainWindow();
	void createInventoryWindow();

	CGUIrichTextPanel * spawnPopText(bool modal);

	void createTextStyles();

	void hide(bool onOff);
	
	void reset();

	void tempText(bool onOff, int winId);

	void update(float dT);

	void pause(bool isOn);

	void mouseOverHotText(int hotId);
	void displayNarrativeChoice(int hotId);
	void mouseWheelHotText(int hotID, int direction);

	glm::i32vec2 randomWindowPos();

	void processMessageQueue();

	void GUIcallback(CGUIbase* sender, CMessage& msg);

	void onHotTextMouseOver(int hotId);

	void positionPopupWindow(CGUIrichTextPanel* popupWin);

	//unsigned int textWindowID;
	//CGUIrichText* textWindow;
	CGUIrichTextPanel* mainTextPanel;
	unsigned int mainTextWindowID;

	CGUIrichTextPanel* invPanel;
	unsigned int invPanelID;


	CGUIrichTextPanel* popupPanel;
	unsigned int popupPanelID;
	unsigned int popupTextID;

	TtextTheme normalTheme; ///<Groups normal styles for rich text.
	TtextTheme smallNormalTheme; ///<Groups normal styles for rich text, smaller font size.

private:
	CTigVM* pVM;
	CGUIrichTextPanel* pMenuWindow;

	C3DtestApp* pApp;

	CFont* popHeaderFont;
	CFont* popBodyFont;

	int playerId; ///<Id of the player object.
	int clickId; ///<Id of the click message.
	int examId; ///<Id of the examine message
	int moveToId; ///<Id of the moveTo message
	int showPlayerOpsId; 

	glm::i32vec2 lastMouseOverPos;
	glm::i32vec2 lastMenuCorner;

	

	glm::vec4 darkGray;
	glm::vec4 choiceColour;
	glm::vec4 white;
	glm::vec4 hottextColour;
	glm::vec4 hottextSelectedColour;

	std::vector<TObjWindow> objWindows; //TO DO: deprecating, remove
	std::vector<TObjWindow> popupWindows;

	CFont mainFont;
	CFont mainFontBold;

	CLog transcript; ///<For logging output to main window.

	int currentVariant; ///<Index no. of the hot text variant currently selected

	std::mt19937 randEngine; ///<Random number engine.

	std::queue<TvmAppMsg> messages; 

	int currentMouseOverObj; ///<Most recent object a hot text function call was made on. -1 when mouse leaves hot text.
};





const int popMenuId = 5000;
const int popObjWinId = 5001;

//tig script constants TO DO: can read these from the bytecode
const int mainWin = 0;
const int invWin = 1;
const int menuWin = 2;
const int combatWin = 3;
const int msgRoomChange = 5000;
