#include "playerHexObj.h"

#include "utils/log.h"

#include "hexItem.h"

#include "gameState.h"



CPlayerObject::CPlayerObject() {
	psu = new CPowerSupply();

	messageBus.setHandler<CGetPlayerPos>(this, &CPlayerObject::onGetPlayerPos);
	messageBus.setHandler<CSetPlayerAction>(this, &CPlayerObject::onSetPlayerAction);
	messageBus.setHandler<CTakeItem>(this, &CPlayerObject::onTakeItem);
	messageBus.setHandler<CGetPlayerObj>(this, &CPlayerObject::onGetPlayerObj);
	messageBus.setHandler<CPlayerSeen>(this, &CPlayerObject::onPlayerSeen);

	tmpHP = 12;
	actionPoints = 6;

	viewField.centre = CHex(0);
	viewField.setField(10);

	APlabel = gui::addLabel(std::to_string(actionPoints), 600, style::mainWinCtrlBorder);
	APlabel->setTextColour(style::uiWhite);
	APlabel->setSize(style::labelPresets[1]);
	APlabel->setFont(style::themes2.at("gameTheme").styles.at("mainHeader").font);

}

CPlayerObject::~CPlayerObject() {
	gui::removeControl(APlabel);
}

/** Player has pressed or released the action key. */
void CPlayerObject::onActionKey(bool pressed) {

	if (pressed && world.getTurnPhase() == playerPhase) {
		//TO DO: for now assume move, may/should be other possibilities later


		if (action == tig::actMoveTo) { //if we're already moving we want to stop
			if (!travelPath.empty())
				travelPath.erase(travelPath.begin() + 1, travelPath.end());
			return;
		}

		if (hexPosition == world.cursorPos  )
			return;
		CGetTravelPath pathRequest(hexPosition, world.cursorPos,true);
		send(pathRequest);
		if (pathRequest.travelPath.empty())
			return;

		travelPath = pathRequest.travelPath;

		setActionMoveTo(world.cursorPos);
		//world.setTurnPhase(actionPhase);
	}
	else {
		return;
		/*if (action == tig::actMoveTo) {
			if (!travelPath.empty())
				travelPath.erase(travelPath.begin() + 1, travelPath.end());


		}*/


	}
}




void CPlayerObject::setActionMoveTo(CHex& hex) {
	action = tig::actMoveTo;
	targetHex = hex;
//	CGetTravelPath pathRequest(hexPosition, targetHex);
//	send(pathRequest);
//	travelPath = pathRequest.travelPath;
	if (travelPath.size() > movePoints2)
		travelPath.resize(movePoints2);
	destHexClaimed = false;
	blockedFor = 0;
	CAddActor msg(this, actionSerial);
	send(msg);
}

bool CPlayerObject::update(float dT) {
	this->dT = dT;
	CHex oldPosition = hexPosition;

	switch (action) {
	case tig::actMoveTo:
	case tig::actPlayerMove:
		if (navigatePath(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actMelee:
		if (meleeAttack(dT)) {
			hitTarget();
			action = tig::actNone;
		}
		break;

	case tig::actShoot:
		if (shootTarget(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actNone:
		return resolved;
	}

	//TO DO: may not need as well as CActorMovedHex
	if (hexPosition != oldPosition) {
		CPlayerNewHex msg(hexPosition);
		send(msg);
	}

	return unresolved;
}





/** Deliver melee damage to our current target. */
void CPlayerObject::hitTarget() {
	if (actionTarget == NULL)
		return;

	CFindPowerUser msg(this);
	send(msg);
	int damage = msg.power;

	actionTarget->receiveDamage(*this, damage);
}



void CPlayerObject::draw() {
	//for (auto hex : viewField.visibleHexes)
	//	hexRendr->highlightHex(hex);

	CHexObject::draw();
}


void CPlayerObject::takeItem(CGameHexObj& item) {

	CRemoveEntity msg(&item);
	send(msg);

	playerItems.push_back(&item);

	callTig(tig::onTake, item);
}

void CPlayerObject::showInventory() {
	callTig(tig::onInventory);
}

void CPlayerObject::dropItem(int itemNo) {	
	ITigObj* item = callTigObj(tig::onDrop, itemNo);
	CGameHexObj* gameObj = (CGameHexObj*)item->getCppObj();
	auto playerItem = std::find(playerItems.begin(), playerItems.end(), gameObj);

	CDropItem msg((CHexItem *) *playerItem, hexPosition);
	send(msg);
	playerItems.erase(playerItem);
}

void CPlayerObject::equipItem(int itemNo) {
	callTigObj(tig::onEquip, itemNo);
}

/** Respond to left-click/primary action in power mode. */
void CPlayerObject::leftClickPowerMode() {
	CReserveNextPower msg(this);
	send(msg);
}

void CPlayerObject::updateActionPoints(int change) {
	actionPoints += change;
	APlabel->setText(std::to_string(actionPoints));
}


void CPlayerObject::onTurnBegin() {
	liveLog << "\nPlayer turn begin";
	psu->topUp();

	psu->updateDisplay();

	actionPoints = 6;
}

/**Tidy up player state ready for the next round. */
void CPlayerObject::onTurnEnd() {

	if (world.onscreenRobotAction == true) {
		if (action == tig::actMoveTo) { //if we're already moving we want to stop
			travelPath.clear();
			action = tig::actNone;
		}

	}


	psu->onTurnEnd();
	psu->updateDisplay();
}

void CPlayerObject::onGetPlayerPos(CGetPlayerPos& msg) {
	msg.position = hexPosition;
}

void CPlayerObject::onSetPlayerAction(CSetPlayerAction& msg) {
	//TO DO: redundant, scrap
}

void CPlayerObject::onTakeItem(CTakeItem& msg) {
	takeItem(*msg.item);
}

void CPlayerObject::onGetPlayerObj(CGetPlayerObj& msg) {
	msg.playerObj =  this;
}

void CPlayerObject::onPlayerSeen(CPlayerSeen& msg) {
	if (action == tig::actMoveTo) {
		if (!travelPath.empty()) {
			travelPath.clear();
			action = tig::actNone;
		}
	}
}


void CPlayerObject::deathRoutine() {
	std::string deathLog = "\nYou were killed!";

	CSendText msg(combatLog, deathLog);
	send(msg);

	CKill killMsg(this);
	send(killMsg);
}

void CPlayerObject::receiveDamage(CGameHexObj& attacker, int damage) {
	CFindPowerUser msg(&attacker);
	send(msg);

	damage -= msg.power;

	if (damage <= 0) {
		CSendText block(combatLog, "\nAttack from " + attacker.getName()
			+ " blocked!");
		send(block);
		return;
	}

	CGameHexObj::receiveDamage(attacker, damage);
}

int CPlayerObject::getMissileDamage() {
	//get assigned power from QPS
	CFindPowerUser msg(this);
	send(msg);

	return msg.power;
}

/** Called when player has arrived at a new hex.*/
void CPlayerObject::onMovedHex() {
	CActorMovedHex msg(hexPosition, this);
	send(msg);

	updateViewField();

	updateActionPoints(-1);

	if (actionPoints == 0) {
		CPlayerTurnEnd msg;
		send(msg);
	}
}


void CPlayerObject::updateViewField() {
	//update view field
	viewField.update(hexPosition);
	CCalcVisionField calcFieldMsg(hexPosition, viewField.ringHexes, true);
	send(calcFieldMsg);

	std::vector<CHex> unvisibledHexes;
	for (auto hex : viewField.visibleHexes) {
		if (std::find(calcFieldMsg.visibleHexes.begin(), calcFieldMsg.visibleHexes.end(),
			hex) == calcFieldMsg.visibleHexes.end()) {
			unvisibledHexes.push_back(hex);
		}
	}

	viewField.visibleHexes = calcFieldMsg.visibleHexes;

	//update fog of war
	CUpdateFog fogMsg(calcFieldMsg.visibleHexes, unvisibledHexes);
	send(fogMsg);

}

/** Respond to player move instruction. */
void CPlayerObject::moveCommand(TPlayerMoveDir dir) {
	if (travelDir == moveNone) {
		switch (dir) {
		case moveEast: moveDest = getNeighbour(hexPosition, hexEast); break;
		case moveWest: moveDest = getNeighbour(hexPosition, hexWest); break;
		case moveNE: moveDest = getNeighbour(hexPosition, hexNE); break;
		case moveSE: moveDest = getNeighbour(hexPosition, hexSE); break;
		case moveNW: moveDest = getNeighbour(hexPosition, hexNW); break;
		case moveSW: moveDest = getNeighbour(hexPosition, hexSW); break;
		case moveNorth:
		case moveSouth: {
			startNorthSouthMove(dir);
			break;
		}
		case moveNS2: {
			moveDest = moveDest2;
			break;
		}
		} //end switch

		//check if move possible
		if (world.isBlocked(hexPosition, moveDest)) {
			if (dir == moveNS2)
				travelDir = moveNS2blocked;
			return;
		}

		//find angle to turn to
		rotation = hexAngle(hexPosition, moveDest);
		buildWorldMatrix();

		travelDir = dir;
	}

}

void CPlayerObject::startNorthSouthMove(TPlayerMoveDir dir) {
	northSouthKeyReleased = false;
	THexDir eastRoute;	THexDir westRoute;
	if (dir == moveNorth) {
		eastRoute = hexNE;
		westRoute = hexNW;
	}
	else {
		eastRoute = hexSE;
		westRoute = hexSW;
	}

	CHex alt;
	if (rotation < M_PI_2 || rotation > M_PI + M_PI_2) {
		moveDest = getNeighbour(hexPosition, eastRoute);
		alt = getNeighbour(hexPosition, westRoute);
	}
	else {
		moveDest = getNeighbour(hexPosition, westRoute);
		alt = getNeighbour(hexPosition, eastRoute);
	}
	if (world.isBlocked(hexPosition, moveDest))
		moveDest = alt;

	if (dir == moveNorth) 
		moveDest2 = hexPosition + CHex(1, 1, -2);
	else
		moveDest2 = hexPosition + CHex(-1, -1, 2);
}

/** Player released up or down key - useful to know so that we don't 
	continue a north or south movement. */
void CPlayerObject::onVerticalKeyRelease() {
	northSouthKeyReleased = true;
}


void CPlayerObject::update2(float dT) {
	this->dT = dT;

	switch (travelDir) {
	case moveNone:
		break;
	case  moveNS2blocked: {
		if (northSouthKeyReleased) //stops a blocked secondary NS move becoming another NS start
			travelDir = moveNone;
		break;
		}
	default:
		moveReal();
		if (hexPosition == moveDest) {
			if ((travelDir == moveNorth || travelDir == moveSouth) && !northSouthKeyReleased) {
				travelDir = moveNone;
				moveCommand(moveNS2);
			}
			else
				travelDir = moveNone;
		}
	}
}

/** Move realtime toward the destination hex, unless we reach it. */
void CPlayerObject::moveReal() {
	glm::vec3 dest = cubeToWorldSpace(moveDest);
	glm::vec3 travel = dest - worldPos;

	glm::vec3 moveVec = glm::normalize(travel) * playerMoveSpeed * dT;

	float remainingDist = glm::length(travel);

	if (glm::length(moveVec) > remainingDist) {
		worldPos = dest;
		setPosition(moveDest);

		onMovedHex();
		return;
	}

	worldPos += moveVec;
	buildWorldMatrix();
}




