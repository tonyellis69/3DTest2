#include "powerSupply.h"
#include "utils/log.h"

unsigned int CPowerUser::userIds = 0;

CPowerSupply::CPowerSupply() {
	numSlots = 6;
	maxPower = 10;
	nextId = 1;
}

void CPowerSupply::topUp() {

	std::uniform_int_distribution<int> d{ 1,maxPower };
	for (int x = powerSlots.size(); x < numSlots; x++) {
		TPowerBlob blob = { d(randEngine) };
		blob.id = nextId++;
		powerSlots.push_back(blob);
	}
}

/** Show current power slot status. */
void CPowerSupply::updateDisplay() {
	liveLog << "\nslots: ";
	for (auto slot : powerSlots) {
		if (slot.reservedBy ==  assigneeNone) {
			liveLog << slot.power << " ";	
		}
		else
			liveLog << "[" << slot.power << "] ";
	}

}

/** Return the value of the next available power in the queue. */
int CPowerSupply::readNextPower() {
	for (auto slot : powerSlots) {
		if (slot.reservedBy == assigneeNone) {
			return slot.power;
		}
	}
}

/** Return the id of the next available power in the queue, 
	and record that it's reserved. */
unsigned int CPowerSupply::reserveNextPower() {
	//TO DO: record who's getting this power*;

	for (auto& slot : powerSlots) {
		if (slot.reservedBy == assigneeNone) {
			slot.reservedBy = assigneeTmp;//*
			liveLog << "\nslot id " << slot.id << " reserved.";
			updateDisplay();
			return slot.id;
		}
	}

}

/** Return the power value of this reserved power blob. */
int CPowerSupply::getPowerValue(unsigned int id) {
	for (auto slot : powerSlots) {
		if (slot.id == id)
			return slot.power;
	}
	liveLog << "\nPanic!";
}

/** Tidy up the state, ready for the next turn. */
void CPowerSupply::onTurnEnd() {
	//if (inUse > 0 && inUse <= powerSlots.size()) {
	//	powerSlots.erase(powerSlots.begin(), powerSlots.begin() + inUse);
	//}

	////powerSlots.insert(powerSlots.begin(), returnedPower.begin(),returnedPower.end());
	//for (auto power : returnedPower) {
	//	TPowerBlob returnedBlob = { power };
	//	powerSlots.insert(powerSlots.begin(), returnedBlob);
	//}
	//returnedPower.clear();

	topUp();
}

/** Free this reserved power blob for future use. */
void CPowerSupply::unreservePower(unsigned int id) {
	for (auto& slot : powerSlots) {
		if (slot.id == id)
			slot.reservedBy = assigneeNone;
	}
}

/** Remove this power blob from the queue, returning its power value. */
int CPowerSupply::consumePower(unsigned int id) {
	liveLog << "\nasked to consume id " << id;
	for (auto slot = powerSlots.begin(); slot != powerSlots.end(); slot++) {
		if (slot->id == id) {
			int power = slot->power;
			powerSlots.erase(slot);
			return power;
		}
	}
}

/** Record this devices's request for power. */
void CPowerSupply::requestPower(CPowerUser* requester) {
	for (auto request : powerRequests) {
		if (request->userId == requester->userId)
			return;
	}
	powerRequests.push_back(requester);
}

/** Remove this device's request for power. */
void CPowerSupply::removeRequest(CPowerUser* user) {
	for (auto request = powerRequests.begin(); request != powerRequests.end(); request++) {
		if ((*request)->userId == user->userId) {
			powerRequests.erase(request);
			liveLog << "\nRequest removed!";
			return;
		}
	}
}

/** Return the id of the best available power for this power user. */
unsigned int CPowerSupply::getBestPower(CPowerUser* user) {
	//is this user already on the list of requesters?
	//if not, we need to expand the range of available
	//blobs by one.

	int accessibleBlobs = powerRequests.size();
	auto search = std::find_if(powerRequests.begin(), powerRequests.end(),
		[user](CPowerUser* x) { return x->userId == user->userId; });
	if (search == powerRequests.end())
		accessibleBlobs += 1;

	int nextFreeBlob = -1;
	std::vector<TPowerBlob> orderedAvailableBlobs;
	for (int b = 0; b < accessibleBlobs; b++) {
		orderedAvailableBlobs.push_back(powerSlots[b]);
		if (nextFreeBlob < 0 && powerSlots[b].reservedBy == assigneeNone)
			nextFreeBlob = powerSlots[b].id;
	}

	std::sort(orderedAvailableBlobs.begin(), orderedAvailableBlobs.end(),
		[](TPowerBlob const& a, TPowerBlob const& b) { return a.power < b.power; });

	switch (user->autoPower) {
	case autoNone: return 0; 
	case autoCurrent: return nextFreeBlob;
	case autoLow: return orderedAvailableBlobs[0].id;
	case autoHigh:
	case autoBest:  return orderedAvailableBlobs[accessibleBlobs - 1].id;
	}

}

/** Assign power ids to each requester, according to their requirements. */
void CPowerSupply::autoAssign() {
	int accessibleBlobs = powerRequests.size();
	std::vector<TPowerBlob> orderedAvailableBlobs;
	for (int b = 0; b < accessibleBlobs; b++) {
		orderedAvailableBlobs.push_back(powerSlots[b]);
	}

	std::sort(orderedAvailableBlobs.begin(), orderedAvailableBlobs.end(),
		[](TPowerBlob const& a, TPowerBlob const& b) { return a.power < b.power; });

	std::sort(powerRequests.begin(), powerRequests.end(),
		[](CPowerUser* a, CPowerUser* b) { return a->autoPower > b->autoPower; });


	//first, remove any blobs reserved by the player choosing autoCurrent:
	for (auto& request : powerRequests) {
		if (request->autoPower == autoCurrent && request->powerId != 0) {

			for (auto blob = orderedAvailableBlobs.begin(); blob != orderedAvailableBlobs.end(); blob++)
				if (blob->id == request->powerId) {
					orderedAvailableBlobs.erase(blob);
					break;
				}

		}

	}

	
	//now for each request, assign the best power blob
	int bestId;
	for (auto& request : powerRequests) {
		switch (request->autoPower) {
		case autoNone: bestId = 0; break;
		case autoCurrent: if (request->powerId == 0) {
			bestId = orderedAvailableBlobs.back().id; //best of what's left
		} else bestId = request->powerId; break;
		case autoLow: bestId = orderedAvailableBlobs[0].id; break;
		case autoHigh:
		case autoBest:  bestId = orderedAvailableBlobs.back().id; break;
		}

		//remove that id from contention:
		for (auto blob = orderedAvailableBlobs.begin(); blob != orderedAvailableBlobs.end(); blob++)
			if (blob->id == bestId) {
				orderedAvailableBlobs.erase(blob);
				break;
			}

		request->powerId = bestId;
	}
}
