#pragma once

#include "vm.h"
#include "UI\GUIrichText.h"
#include "UI\GUIpopMenu.h"
#include "UI\GUIrichTextPanel.h"
#include "localHotList.h"

struct THotTextRec {
	std::string text;
	int id;
};

enum TPopAction { popDoNothing, popTake, popDrop, popExamine };
struct TPopChoice {
	std::string actionText;
	TPopAction action;
};
 


enum TMoveDir {moveNorth, moveNE, moveEast, moveSE, moveSouth, moveSW, moveWest,
	moveNW, moveUp, moveDown, moveIn, moveOut };

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
	void init();
	void findMoveToIds();
	void findTreeIds();
	void roomDescription();
	void start();
	void addHotText(std::string& text, int id);
	void processText(string  text);
	std::string markupHotText(std::string& text);
	void hotTextClick(int messageId, glm::i32vec2 mousePos);
	void inventoryClick(int messageId, const glm::i32vec2& mousePos);

	void changeRoom(int direction);
	void take(int itemId);
	void drop(int item);
	void examine(int objId);
	int child(int parent);
	int sibling(int object);
	int parent(int childNo);
	bool objectInLoop(int parent, int & child);
	void move(int obj, int dest);
	void refreshInvWindow();
	void refreshLocalList();
	void objectClick(int objId, const glm::i32vec2& mousePos);
	void appendChoicesToPopup();
	void showPopupMenu(const glm::i32vec2& mousePos);
	std::string makeHotText(std::string text, int idNo);
	void popupSelection(int choice, glm::i32vec2& mousePos);
	std::string cap(std::string text);



	void setMainBodyStyle( CFont& font, const glm::vec4& colour);
	void setInvBodyStyle(CFont& font, const glm::vec4& colour);
	void setPopBodyStyle(CFont& font, const glm::vec4& colour);
	void setPopHeaderStyle(CFont& font, const glm::vec4& colour);

private:
	CTigVM* pVM;
	CGUIrichText* pTextWindow;
	CGUIrichText* pInvWindow;
	CGUIrichText* currentTextWindow;
	CGUIrichTextPanel* popControl;

	CFont* popHeaderFont;
	CFont* popBodyFont;

	CTigVar currentRoom; ///<Always stores the address of the room the player is in.
	int currentRoomNo; ///<Always stores the object index of the room the player is in.
	int playerId; ///<Id of the player object.

	std::vector<THotTextRec> hotTextList;

	int moveToIds[12]; ///Convenient store for movement member ids.
	int parentId, childId, siblingId; ///<Tree member ids;

	CLocalHotList localHotList; ///<Tracks hot text for objects currently in scope.
	int clickedHotText; ///<Id of currently clicked hot text.
	int clickedObj; ///<Id of currently clicked object;
	std::vector<TPopChoice> popChoices; ///<Tracks choices available on the popup meny.
	glm::i32vec2 currentMousePos;

	TtextStyle mainBodyStyle; ///<Text style for main window body text.
	TtextStyle invBodyStyle;
	TtextStyle popBodyStyle;
	TtextStyle popHeaderStyle;

	glm::vec4 hottextColour;
	glm::vec4 hottextSelectedColour;
};


