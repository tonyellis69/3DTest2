#pragma once

#include <random>
#include <queue>

//#include "3DtestApp.h"
#include "vmApp.h"
#include "UI\GUIrichText.h"
//#include "UI\GUIpopMenu.h"
#include "GUIrichTextPanel.h"
#include "localHotList.h"
#include "GUIgamePanel.h"
//#include "GUIdistributor.h"
//#include "GUIhealthPanel.h"



 
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
	void onVMaccumulatorUpdate(int powerUpdate);
	void onVMpowerCellUpdate(int powerUpdate);
	void onVMhpChange(int hpUpdate);
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

	void popupWindowClick(unsigned int hotId, glm::i32vec2 mousePos, CGUIrichTextPanel* popUp);

	void deletePopupWindow(int id);

	void vmMessage(const std::string& p1, int p2);

	void queueMsg(TvmAppMsg& msg);

	void createMainWindow();
	void createInventoryWindow();

	CGUIrichTextPanel * spawnPopText(bool modal);

	
	void createDistributor();
	void createHealthPanel();

	void createTextStyles();

	void hide(bool onOff);
	
	void reset();

	bool tempText(bool onOff, int winId);

	void update(float dT);

	void pause(bool isOn);

	void onHotTextChange(int hotId);
	void handleChoiceText(int hotId);
	bool displayNarrativeChoice(std::string& choiceText);
	void mouseWheelHotText(int hotID, int direction);

	glm::i32vec2 randomWindowPos();

	void processMessageQueue();

	void GUIcallback(CGUIbase* sender, CMessage& msg);

	void popupRequest(TFnCall& fnCall);

	void positionPopupWindow(CGUIrichTextPanel* popupWin);

	void flushMessageQueue();

	void setVMdistributor(int offence, int defence);
	void updateDistributorGUI();

	void updateHealthGUI();

	//unsigned int textWindowID;
	//CGUIrichText* textWindow;
	CGUIrichTextPanel* mainTextPanel;
	unsigned int mainTextWindowID;

	CGUIrichTextPanel* invPanel;
	unsigned int invPanelID;


	CGUIrichTextPanel* popupPanel;
	unsigned int popupPanelID;
	unsigned int popupTextID;

	//CGUIdistributor* distributor;
	//unsigned int distributorID;

	//CGUIhealthPanel* healthPanel;
	//unsigned int healthPanelID;


private:
	CTigVM* pVM;
	CGUIrichTextPanel* pMenuWindow;

	C3DtestApp* pApp;

	CFont* popHeaderFont;
	CFont* popBodyFont;

	int playerId; ///<Id of the player object.
	int mouseoverId; ///<Id of the VM mouseover message.
	int examId; ///<Id of the examine message
	int attemptMoveId; ///<Id of the moveTo message
	int showPlayerOpsId; 
	int gameStateId; ///<Id of the gameState object.
	int tidyModeMask;///<bitmask for tidyMode flag.

	glm::i32vec2 lastMouseOverPos;
	glm::i32vec2 lastMenuCorner;

	

	glm::vec4 darkGray;
	glm::vec4 choiceColour;
	glm::vec4 white;
	glm::vec4 hottextColour;
	glm::vec4 hottextSelectedColour;

	std::vector<TObjWindow> popupWindows;

	CFont mainFont;
	CFont mainFontBold;

	CLog transcript; ///<For logging output to main window.

	int currentVariant; ///<Index no. of the hot text variant currently selected

	std::mt19937 randEngine; ///<Random number engine.

	std::queue<TvmAppMsg> messages; 

	int currentHotId;

	std::string choiceTxt; ///<Most recent choice text, available for reuse.
};



const int noHotText = -1;

const int popMenuId = 5000;
const int popObjWinId = 5001;

//tig script constants TO DO: can read these from the bytecode
const int mainWin = 0;
const int invWin = 1;
const int menuWin = 2;
const int combatWin = 3;
