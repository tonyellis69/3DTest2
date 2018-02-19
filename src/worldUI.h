#pragma once

#include "vm.h"

struct THotTextRec {
	std::string text;
	int memberId;
	};

/** An interface between the player and a game world running on the 
	Tig virtual machine. */
class CWorldUI {
public:
	CWorldUI() {};
	void setVM(CTigVM* vm);
	void init();
	void roomDescription();
	void start();
	void addHotText(std::string& text, int memberId);



private:
	CTigVM* pVM;

	CTigVar currentRoom; ///<Always stores the address of the room the player is in.

	std::vector<THotTextRec> hotTextList;
};