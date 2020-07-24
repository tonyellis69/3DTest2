#include "goalActor.h"

#include "utils/log.h"

/** Give this actor the goal of wandering randomly. */
void CGoalActor::setGoalWander() {
	goal = tig::goalWander;

}

/** Choose an action for this turn, based on the current goal. */
void CGoalActor::chooseTurnAction2() {
	switch (goal) {
	case tig::goalWander: {
		setSimulAction(tig::actMoveTo);

		//are we there yet?
		if (hexPosition == goalDestinationHex) {
			do {
				CRandomHex msg(true);
				send(msg);

				goalDestinationHex = msg.hex;
				CGetTravelPath pathRequest(hexPosition, goalDestinationHex);
				send(pathRequest);
				travelPath = pathRequest.travelPath;
			} while (travelPath.empty());
		}
		else {
			CGetTravelPath pathRequest(hexPosition, goalDestinationHex);
			send(pathRequest);
			travelPath = pathRequest.travelPath;
		}
		liveLog << "\nHeading to " << goalDestinationHex.getCubeVec() << ".";


		if (travelPath.size() > movePoints2)
			travelPath.resize(movePoints2);
		destHexClaimed = false;
		blockedFor = 0;
	}



	}

}
