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



/** Choose an action for this turn, based on the current goal. */
void CGoalActor::chooseTurnAction() {
	switch (goal) {
	case tig::goalWander: 
		chooseWanderAction();
		break;
	case tig::goalAttack: 
		chooseAttackAction();
		break;
	case tig::goalGotoLastSeen:
		chooseGotoLastSeenAction();
		break;


	}

}

void CGoalActor::chooseWanderAction() {
	//setSimulAction(tig::actMoveTo);
	
	if (hexPosition == goalDestinationHex) {
		CRandomHex msg(true);
		send(msg);
		goalDestinationHex = msg.hex;
		
	}
	setActionMoveTo(goalDestinationHex);

}

void CGoalActor::chooseAttackAction() {
	//can we still see our target? Then shoot it
//if not, set goal to pursueToLastSight

	if (canSee(goalTarget)) {
		setActionShoot(goalTarget->hexPosition);
		goalDestinationHex = goalTarget->hexPosition;
	}
	else {
		CHex lastSeen = getLastSeen();
		setGoalGotoLastSeen(lastSeen, goalTarget);
		chooseGotoLastSeenAction();
	}

}

/** Select an action for this goal. */
void CGoalActor::chooseGotoLastSeenAction() {
	//have we reached the last-seen-at hex?
	if (hexPosition != goalDestinationHex) { //no: keep heading to
		setActionMoveTo(goalDestinationHex);
	}
	else { 	//yes? Look for target. For now, by wandering 

		setGoalWander();
		chooseWanderAction();
	}


}
