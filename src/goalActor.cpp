#include "goalActor.h"

#include "utils/log.h"

/** Give this actor the goal of wandering randomly. */
void CGoalActor::setGoalWander() {
	goal = tig::goalWander;
	CRandomHex msg(true);
	send(msg);
	goalDestinationHex = msg.hex;
}


/** Give this actor the goal of killing the given target. */
void CGoalActor::setGoalAttack(CGameHexObj* target) {
	goal = tig::goalAttack;
	goalTarget = target;
}

/** Initiate the goal of going to the hex where a target was last seen.*/
void CGoalActor::setGoalGotoLastSeen(CHex& dest, CGameHexObj* target) {
	goal = tig::goalGotoLastSeen;
	goalTarget = target;
	goalDestinationHex = dest;
}

int CGoalActor::getGoal() {
	return goal;
}








