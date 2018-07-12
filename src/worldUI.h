#pragma once

//#include "3DtestApp.h"
#include "vm.h"
#include "UI\GUIrichText.h"
#include "UI\GUIpopMenu.h"
#include "UI\GUIrichTextPanel.h"
#include "localHotList.h"

struct THotTextRec {
	std::string text;
	int msgId;
	int objId;
};

enum TPopAction { popDoNothing, popTake, popDrop, popExamine,
	popPush};
struct TPopChoice {
	std::string actionText;
	int action;
};
 
struct TObjWindow {
	CGUIrichTextPanel* win;
	CObjInstance* obj;
};

enum TMoveDir {moveNone, moveNorth, moveNE, moveEast, moveSE, moveSouth, moveSW, moveWest,
	moveNW, moveUp, moveDown, moveIn, moveOut };

class C3DtestApp;

/** An interface between the player and a game world running on the 
	Tig virtual machine. */
class CWorldUI {
public:
	CWorldUI() { };
	void setVM(CTigVM* vm);
	void setTextWindow(CGUIrichText* txtWin);
	void setInventoryWindow(CGUIrichText * invWin);
	void setCurrentWindow(CGUIrichText * pWin);
	void setPopupTextWindow(CGUIrichTextPanel * pPopPanel);
	void setGameApp(C3DtestApp* app);
	void init();
	void findMoveToIds();
	void findTreeIds();
	void findClassIds();
	void findVerbIds();
	void roomDescription();
	std::string markupInitialText(CObjInstance* obj);
	void start();
	void addHotText(std::string& text, int msgId, int objId);
	void appendText(std::string& text, int window);
	std::string markupExits(std::string& text);
	void hotTextClick(int msgId, int objId, glm::i32vec2 mousePos);
	void inventoryClick(int msgId, int objId, const glm::i32vec2& mousePos);

	void changeRoom(int direction);
	TMoveDir calcBackDirection(int moveId);
	void take(int itemId);
	void drop(int item);
	void examine(int objId);
	void fillObjectWindow(CGUIrichTextPanel* pop, CObjInstance* obj);
	void push(int objId);
	void climb(int objId);
	void purge(int memberId, int objId);
	void clearWindow(int window);

	int child(int parent);
	CObjInstance * child(CObjInstance * parentObj);
	int sibling(int object);
	CObjInstance * sibling(CObjInstance * obj);
	int parent(int childNo);
	CObjInstance * parent(CObjInstance * child);
	bool objectInLoop(int parent, int & child);
	bool objectInLoop(CObjInstance * parent, CObjInstance * &childObj);
	void move(CObjInstance * obj, CObjInstance * dest);
	//void move(int obj, int dest);
	void refreshInvWindow();
	//void refreshLocalList();
	void objectClick(int objId, const glm::i32vec2& mousePos);
	void appendChoicesToPopup(CGUIrichTextPanel* popControl, int objId);
	void showPopupMenu(CGUIrichTextPanel* popControl, const glm::i32vec2& mousePos);
	std::string makeHotText(std::string text, int msgId, int objId);
	void popupSelection(const int choice, int objId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp);
	std::string cap(std::string text);
	std::string getExitsText(CObjInstance * room);



	void setMainBodyStyle( CFont& font, const glm::vec4& colour);
	void setInvBodyStyle(CFont& font, const glm::vec4& colour);
	void setPopBodyStyle(CFont& font, const glm::vec4& colour);
	void setPopHeaderStyle(CFont& font, const glm::vec4& colour);
	void setHottextColour(const glm::vec4& colour);
	void setHottextSelectColour(const glm::vec4& colour);

private:
	CTigVM* pVM;
	CGUIrichText* pTextWindow;
	CGUIrichText* pInvWindow;
	CGUIrichText* currentTextWindow;
	//CGUIrichTextPanel* popControl;

	C3DtestApp* pApp;

	CFont* popHeaderFont;
	CFont* popBodyFont;

	CObjInstance* currentRoom; ///<Always stores the address of the room the player is in.
	CObjInstance* player;
	int currentRoomNo; ///<Always stores the object index of the room the player is in.
	int playerId; ///<Id of the player object.

	int corridorClassId;
	int roomClassId;
	int staticClassId;
	int sceneryClassId;
	int supporterClassId;

	int takeId;
	int dropId;
	int examineId;
	int pushId;
	int climbId;

	std::vector<THotTextRec> hotTextList;

	int moveToIds[13]; ///Convenient store for movement member ids.
	int parentId, childId, siblingId; ///<Tree member ids;

	//CLocalHotList localHotList; ///<Tracks hot text for objects currently in scope.
	bool bodyListedExits[13]; ///<Any exit directions listed in body copy.

	int clickedHotText; ///<Id of currently clicked hot text.
	std::vector<TPopChoice> popChoices; ///<Tracks choices available on the popup meny.
	glm::i32vec2 currentMousePos;
	glm::i32vec2 lastMenuCorner;

	TtextStyle mainBodyStyle; ///<Text style for main window body text.
	TtextStyle invBodyStyle;
	TtextStyle popBodyStyle;
	TtextStyle popHeaderStyle;

	glm::vec4 hottextColour;
	glm::vec4 hottextSelectedColour;

	TMoveDir backDirection; ///<Direction the player came. 

	std::vector<TObjWindow> objWindows;
};


const int popMenuId = 5000;
const int popObjWinId = 5001;

const int mainWin = 0;
const int invWin = 1;
