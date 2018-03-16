#pragma once

#include "vm.h"
#include "UI\GUIrichText.h"

struct THotTextRec {
	std::string text;
	int id;
};



enum TMoveDir {moveNorth, moveNE, moveEast, moveSE, moveSouth, moveSW, moveWest,
	moveNW, moveUp, moveDown, moveIn, moveOut };

/** An interface between the player and a game world running on the 
	Tig virtual machine. */
class CWorldUI {
public:
	CWorldUI() {};
	void setVM(CTigVM* vm);
	void setTextWindow(CGUIrichText* txtWin);
	void setInventoryWindow(CGUIrichText * invWin);
	void setCurrentWindow(CGUIrichText * pWin);
	void init();
	void findMoveToIds();
	void findTreeIds();
	void roomDescription();
	void start();
	void addHotText(std::string& text, int id);
	void processText(string  text);
	void writeRichText(string text, CGUIrichText * pWin);
	void markupHotText(std::string& text);
	void hotTextClick(int messageId);
	void inventoryClick(int messageId);

	void changeRoom(int direction);
	void take(int itemId);
	void drop(int item);
	int child(int parent);
	int sibling(int object);
	int parent(int childNo);
	bool objectInLoop(int parent, int & child);
	void move(int obj, int dest);
	void refreshInvWindow();

private:
	CTigVM* pVM;
	CGUIrichText* pTextWindow;
	CGUIrichText* pInvWindow;
	CGUIrichText* currentTextWindow;

	CTigVar currentRoom; ///<Always stores the address of the room the player is in.
	int currentRoomNo; ///<Always stores the object index of the room the player is in.
	int playerId; ///<Id of the player object.

	std::vector<THotTextRec> hotTextList;

	int moveToIds[12]; ///Convenient store for movement member ids.
	int parentId, childId, siblingId; ///<Tree member ids;

	std::vector<int> roomItems; ///<Ids of any items, for hot text crosschecking.
};


