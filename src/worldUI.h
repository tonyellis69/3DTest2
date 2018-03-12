#pragma once

#include "vm.h"
#include "UI\GUIrichText.h"

struct THotTextRec {
	std::string text;
	int memberId;
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
	void init();
	void findMoveToIds();
	void findTreeIds();
	void roomDescription();
	void start();
	void addHotText(std::string& text, int memberId);
	void processText(string & text);
	void markupHotText(std::string& text);
	void hotTextClick(int messageId);

	void moveTo(int direction);

	int child(int parent);

	int sibling(int object);

	int parent(int childNo);

	bool objectInLoop(int parent, int & child);

private:
	CTigVM* pVM;
	CGUIrichText* pTextWindow;

	CTigVar currentRoom; ///<Always stores the address of the room the player is in.
	int currentRoomNo; ///<Always stores the object index of the room the player is in.

	std::vector<THotTextRec> hotTextList;

	int moveToIds[12]; ///Convenient store for movement member ids.
	int parentId, childId, siblingId; ///<Tree member ids;
};


