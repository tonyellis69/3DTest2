#include "hexWorld.h"

#include <string>

#include "utils/log.h"

#include "gameTextWin.h"

#include "gameState.h"

CGameHexObj nullGameHexObj;

CHexWorld::CHexWorld() {
	CHexObject::setHexRenderer(&hexRenderer);
	CGridObj::setHexRenderer(&hexRenderer);
	
	messageBus.setHandler<CAddActor>(this, &CHexWorld::onAddActor);
	messageBus.setHandler<CShootAt>(this, &CHexWorld::onShootAt);
	messageBus.setHandler<CDropItem>(this, &CHexWorld::onDropItem);
	messageBus.setHandler<CRemoveEntity>(this, &CHexWorld::onRemoveEntity);
	messageBus.setHandler<CCreateGroupItem>(this, &CHexWorld::onCreateGroupItem);
	messageBus.setHandler<CMissileHit>(this, &CHexWorld::onMissileHit);
	messageBus.setHandler<CKill>(this, &CHexWorld::onKill);
	messageBus.setHandler<CDiceRoll>(this, &CHexWorld::onDiceRoll);
	messageBus.setHandler<CPlayerNewHex>(this, &CHexWorld::onPlayerNewHex);
	messageBus.setHandler<CActorMovedHex>(this, &CHexWorld::onActorMovedHex);
	messageBus.setHandler<CPlayerTurnEnd>(this, &CHexWorld::onPlayerTurnEnd);


	subscribe(&world);

	hexPosLbl = new CGUIlabel2(10, 10, 550, 30);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}


void CHexWorld::setVM(Ivm* pVM) {
	vm = pVM;
	mapMaker.attachVM(pVM);
	setTigObj(vm->getObject(tig::IHexWorld)); 
	CGameTextWin::pVM = pVM;
}

/**	Load a multipart mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, const std::string& fileName) {
	hexRenderer.loadMesh(name, fileName);
}

/** Create a map using the data in the given Tig file. */
void CHexWorld::makeMap(ITigObj* tigMap) {
	do {
		map = mapMaker.makeMap(tigMap);
		if (!map->isValidPath(CHex(-13, 5, 8), CHex(13, -4, -9))) {
			delete map;
		}
		else break;
	} while (true);

	map->setMessageHandlers();

	for (auto entity : map->entities)
		subscribe(entity);

	hexRenderer.setMap(map);
}

void CHexWorld::deleteMap() {
	for (auto entity : map->entities)
		unsubscribe(entity);
	delete map;
}


/** Required each time we restart. */
void CHexWorld::startGame() {
	hexRenderer.setMap(map);
	world.setMap(map);
	
	//create new player object
	playerObj = new CPlayerObject();
	playerObj->setLineModel("player");
	//map->add(playerObj, CHex(-13, 5, 8));
	map->add(playerObj, CHex(0, -3, 3));
	world.player = playerObj;


	playerObj->setTigObj(vm->getObject("player"));
	map->entities.push_back(playerObj);
	subscribe(playerObj);
	playerObj->updateViewField();
	alertEntitiesInPlayerFov();



	entitiesToDraw.assign(map->entities.rbegin(),map->entities.rend());

	world.setTurnPhase(playerPhase);

	if (hexCursor == NULL)
		createCursorObject();
	hexCursor->visibleToPlayer = true;

	CSendText msg(combatLog, "", true);
	send(msg);

	map->updateBlocking();
	hexRenderer.updateHexShaderBuffer();

	beginNewTurn(); //NB!!! Repeats some of the stuff above
}

void CHexWorld::update2(float dT) {
	world.player->update2(dT);
}


void CHexWorld::moveCamera(glm::vec3& direction) {
	float camSpeed = 6.0f * dT;
	glm::vec3 vector = direction *= camSpeed;
	hexRenderer.moveCamera(vector);
}

/** Called when a key is pressed. */
void CHexWorld::onKeyDown(int key, long mod) {
	//temp user interface stuff!
	if (key == 'I')
		playerObj->showInventory();

	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
		int item = key - GLFW_KEY_1;
		if (mod == GLFW_MOD_SHIFT) {
			tempGetGroupItem(item);
		}
		else if (mod == GLFW_MOD_CONTROL)
			playerObj->equipItem(item);
		else
			playerObj->dropItem(item);
	}

	if (key == 'F') {
		hexRenderer.toggleFollowCam();
		hexRenderer.followTarget(playerObj->worldPos);
	}

	if (key == 'R') {
		hexRenderer.hexLineShader->recompile();
		hexRenderer.lineShader->recompile();
		hexRenderer.hexSolidShader->recompile();
	}

	if (key == 'L') {
		lineOfSight = !lineOfSight;

	}
 
}

void CHexWorld::onMouseWheel(float delta, int key) {
	if (key == GLFW_KEY_LEFT_SHIFT) {
		hexRenderer.pitchCamera(delta);
	}
	else {
		if (key == GLFW_KEY_LEFT_CONTROL)
			hexRenderer.dollyCamera(delta);
		else {
			;
		}
	}
}

void CHexWorld::onMouseMove(int x, int y, int key) {
	if (world.getTurnPhase() == playerDeadPhase) {
		//TO DO: temp! player death should turn off input at the source
		return;
	}

	if (world.getTurnPhase() != playerPhase)
		return;

	if (playerObj->getAction() != tig::actNone)
		return;


	mousePos = { x,y };
	CHex mouseHex = hexRenderer.pickHex(x, y);
	if (mouseHex != hexCursor->hexPosition) {
		CMouseExitHex msg;
		msg.leavingHex = hexCursor->hexPosition;
		notify(msg);
		onNewMouseHex(mouseHex);
	}
}



void CHexWorld::draw() {
	if (world.getTurnPhase() == playerPhase && playerObj->getAction() == tig::actNone) {
		hexRenderer.draw();
		hexCursor->draw();

		for (auto entity : entitiesToDraw)
			entity->draw();
		for (auto gridObj : gridObjects)
			gridObj->draw();

		if (!powerMode)
			hexRenderer.drawPath(&cursorPath, glm::vec4{ 0.6, 0.4, 1, 0.1f }, glm::vec4{ 0.6, 0.4, 1, 0.75f });
	}
	else {
		hexRenderer.draw();


		for (auto entity : entitiesToDraw)
			entity->draw();
		for (auto gridObj : gridObjects)
			gridObj->draw();

		hexRenderer.drawPath(&cursorPath, glm::vec4{ 0.3, 0.2, 1, 0.1f }, glm::vec4{ 0.3, 0.2, 1, 0.75f });
	}


}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	hexRenderer.setCameraAspectRatio(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dT) {
	this->dT = dT;


	removeDeletedEntities();

	updateCameraPosition();

	update2(dT);

	for (auto entity : map->entities)
		entity->frameUpdate(dT);

	if (world.getTurnPhase() == playerPhase) {
		if (resolvingPlayerSerialActions())
			return;
	}


	if (world.getTurnPhase() == robotPhase) {
		if (resolvingGridObjActions())
			return;

		if (resolvingSerialActions())
			return;

		if (resolvingSimulActions()) {
			return;
		}

		//end of action phase
		endTurn();

		beginNewTurn();
	}
}


/** User has triggered the ctrl left mouse  event. */
void CHexWorld::onCtrlLMouse() {
	
}

/** User has released set-defence key. */
void CHexWorld::powerKeyRelease() {
	//for now, just do this
	powerMode = false;
	CSysMsg msg(powerMode);
	send(msg);
}

/** Player has pressed the power mode on/off key. */
void CHexWorld::powerKeyDown() {
	//TO DO: call func on QPS
	//for now, just do this
	powerMode = true;
	CSysMsg msg(powerMode);
	send(msg);
}

/** Player has pressed the enter key. */
void CHexWorld::enterKeyDown() {
	deleteMap();
	simulList.clear();
	serialList.clear();
	entitiesToDraw.clear();

	makeMap(vm->getObject("testRoom"));
	startGame();
}


/////////////public - private devide

void CHexWorld::createCursorObject() {
	hexCursor = new CGameHexObj();
	hexCursor->setLineModel("cursor");
	hexCursor->setPosition(CHex(0, 0, 0));
}


/** Respond to mouse cursor moving to a new hex. */
void CHexWorld::onNewMouseHex(CHex& mouseHex) {
	hexCursor->setPosition(mouseHex);
	if (!lineOfSight)
		cursorPath = map->aStarPath(playerObj->hexPosition, hexCursor->hexPosition, true);
	else
		cursorPath = *hexLine2(playerObj->hexPosition, hexCursor->hexPosition);


	std::stringstream coords; coords << "cube " << mouseHex.x << ", " << mouseHex.y << ", " << mouseHex.z;
	glm::i32vec2 offset = cubeToOffset(mouseHex);
	coords << "  offset " << offset.x << ", " << offset.y;
	glm::i32vec2 index = map->cubeToIndex(mouseHex);
	coords << " index " << index.x << " " << index.y;
	glm::vec3 worldSpace = cubeToWorldSpace(mouseHex);
	coords << " worldPos " << worldSpace.x << " " << worldSpace.y << " " << worldSpace.z;
	coords << " " << map->getHexCube(mouseHex).content;
	//glm::vec2 screenPos = hexRenderer.worldPosToScreen(worldSpace);
	//coords << " " << screenPos.x << " " << screenPos.y;
	coords << " mouse " << mousePos.x << " " << mousePos.y;
	hexPosLbl->setText(coords.str());

	COnCursorNewHex msg;
	msg.newHex = mouseHex;
	notify(msg);
}



/** All entities choose their action for the coming turn. */
void CHexWorld::robotsChooseActions() {
	currentSerialActor = NULL;
	for (auto actor : map->actors) {
		actor->chooseTurnAction();
	}
//	world.setTurnPhase(playerChoosePhase);
}


void CHexWorld::startActionPhase() {
	//world.setTurnPhase(actionPhase);
}

/** Begin a player right-click action. */
void CHexWorld::rightClick() {
	/*if (world.getTurnPhase() != playerChoosePhase )
		return;

	if (powerMode)
		return;*/

}

/** Begin a player left-click action. */
void CHexWorld::leftClick() {
	return;

	if (world.getTurnPhase() != playerPhase)
		return;

	CGameHexObj* obj = getPrimaryObjectAt(hexCursor->hexPosition);

	if (powerMode) {
		obj->leftClickPowerMode();
		return;
	}

	if (obj != &nullGameHexObj) {
		obj->leftClick();
		startActionPhase();
		return;
	}


	//empty hex = shoot
	playerObj->setActionShoot(hexCursor->hexPosition);
	startActionPhase();
	return;
}

void CHexWorld::onActionKey(bool pressed) {
	playerObj->onActionKey(pressed);
}



/** If there are any gridObjects, update the lead one and return true. */
bool CHexWorld::resolvingGridObjActions() {
	for (auto gridObj = gridObjects.begin(); gridObj != gridObjects.end(); gridObj++) {
		if (!(*gridObj)->update(dT)) {
			gridObjects.erase(gridObj);
		}
		return true;
	}
	return false;
}

bool CHexWorld::resolvingPlayerSerialActions() {
	if (playerSerialList.empty())
		return resolved;

	if (currentSerialActor != playerSerialList[0]) {
		currentSerialActor = playerSerialList[0];
		playerSerialList[0]->initAction();
	}

	if (playerSerialList[0]->update(dT) == resolved) {
		playerSerialList.erase(playerSerialList.begin());
		if (playerSerialList.empty()) {
			map->updateBlocking();
			return resolved;
		}
	}

	return unresolved;
}

/** If any entity is performing a serial action, update the lead one and return true. */
bool CHexWorld::resolvingSerialActions() {
	if (serialList.empty())
		return resolved;

	world.onscreenRobotAction = true;

	if (currentSerialActor != serialList[0]) {
		currentSerialActor = serialList[0];
		 serialList[0]->initAction();
	}

	if (serialList[0]->update(dT) == resolved) {
		serialList.erase(serialList.begin());
		if (serialList.empty()) {
			return resolved;
		}
	}

	return unresolved;
}

/** If any entity is performing a simultaneous action, update each one and return true. */
bool CHexWorld::resolvingSimulActions() {
	bool stillResolving = false;

	if (!simulList.empty())
		world.onscreenRobotAction = true;

	for (auto actor = simulList.begin(); actor != simulList.end();) {
		bool resolving = (*actor)->update(dT);
		if (resolving)
			actor++;
		else
			actor = simulList.erase(actor);
		stillResolving |= resolving;
	}


	if (stillResolving)
		return true;

	//world.setTurnPhase(chooseActionPhase);
	playerObj->onTurnEnd(); liveLog << "\nturn end!";
	return false;
}



/** Handle an 'external function call' from Tig. */
int CHexWorld::tigCall(int memberId) {
	std::string strParam;
	switch (memberId) {
	case tig::msgFn : 
		strParam = vm->getParamStr(0);
		liveLog << strParam; break;
	case tig::popupWin :
		strParam = vm->getParamStr(0);
		;// mothballing this, use messaging popupMsg(strParam); break;
	}
	return 1;
}


/** Cope with the player dropping an item. */
void CHexWorld::dropItem(CGameHexObj* item, CHex& location) {
	CGameHexObj* existingItem = getItemAt(location);
	if (existingItem) {
		existingItem->droppedOnBy(*item);
		return;
	}

	item->setPosition(location);
	map->entities.push_back(item);
	map->add(item, location);
}

CGroupItem* CHexWorld::createGroupItem() {
	CGroupItem* groupItem = new CGroupItem();
	groupItem->setLineModel("test");
	groupItem->setTigObj(vm->getObject(tig::CGroupItem));
	map->entities.push_back(groupItem);
	return groupItem;
}

/** Remove this grid object from the grid objects list. */
void CHexWorld::removeGridObj(CGridObj& gridObj) {
	auto removee = std::find(gridObjects.begin(), gridObjects.end(), (CGridObj*)&gridObj);
	if (removee != gridObjects.end())
		gridObjects.erase(removee);
}

/** Return the CItem or CGroupItem object found at this hex, if any. */
CGameHexObj* CHexWorld::getItemAt(CHex& position) {
	for (auto entity : map->entities) {
		if (entity->hexPosition == position && (entity->isTigClass(tig::CItem)
			|| entity->isTigClass(tig::CGroupItem)) )
			return entity;
	}
	return NULL;
}

void CHexWorld::tempGetGroupItem(int itemNo) {
	CGameHexObj* item = getItemAt(hexCursor->hexPosition);
	if (item->isTigClass(tig::CGroupItem)) {
		CGameHexObj*  gotItem = static_cast<CGroupItem*>(item)->removeItem(itemNo);
		playerObj->takeItem(*gotItem);
	}
}

void CHexWorld::removeDeletedEntities() {
	for (auto entity = map->entities.begin(); entity < map->entities.end();) {
		if ((*entity)->deleteMe)
			entity = map->entities.erase(entity);
		else
			entity++;
	}
}

CGridObj* CHexWorld::createBolt() {
	CBolt* bolt = new CBolt();
	bolt->setLineModel(hexRenderer.getLineModel("bolt"));
	gridObjects.push_back(bolt);
	return bolt;
}


/** Add this actor to an action list. */
void CHexWorld::onAddActor(CAddActor& msg) {
	if (msg.actor == playerObj) {
		playerSerialList.push_back(playerObj);
		return;
	}
	//TO DO: kludgy, look for a better way to handle player


	if (msg.addTo == actionSimul)
		simulList.push_back(msg.actor);
	else
		serialList.push_back(msg.actor);
}

/** Called when someone wants to shoot a missile at a target or hex.*/
void CHexWorld::onShootAt(CShootAt& msg) {
	CBolt* boltTmp = (CBolt*)createBolt();
	boltTmp->setPosition(msg.start);
	boltTmp->damage = msg.damage;
	boltTmp->fireAt(msg.attacker, msg.target);
}

void CHexWorld::onDropItem(CDropItem& msg) {
	dropItem(msg.item, msg.location);
}

void CHexWorld::onRemoveEntity(CRemoveEntity& msg) {
	map->removeFromMap(msg.entity);
}

void CHexWorld::onCreateGroupItem(CCreateGroupItem& msg) {
	CGroupItem* groupItem = createGroupItem();
	groupItem->items.push_back(msg.item1);
	groupItem->items.push_back(msg.item2);
	map->removeFromMap(msg.item2);
	groupItem->setPosition(playerObj->hexPosition);
}

/** Handle a missile landing in this hex. */
void CHexWorld::onMissileHit(CMissileHit& msg) {
	//Anything here?
	//if so, damage it
	CGameHexObj* obj = getPrimaryObjectAt(msg.hex);
	if (obj != &nullGameHexObj)
		obj->receiveDamage(*msg.attacker, msg.damage);
}

/** Handle an entity's request to be killed off. */
void CHexWorld::onKill(CKill& msg) {
	killEntity(msg.entity);
}

/** Make psuedo random dice roll and return the result. */
void CHexWorld::onDiceRoll(CDiceRoll& msg) {
	std::uniform_int_distribution<int> d{ 1,msg.die };
	msg.result =  d(randEngine);
	if (msg.die2 != 0)
		msg.result2 = d(randEngine);
}

void CHexWorld::onPlayerNewHex(CPlayerNewHex& msg) {
	notify(msg);
	cursorPath.erase(cursorPath.begin());
	hexRenderer.updateHexShaderBuffer();
}

void CHexWorld::onActorMovedHex(CActorMovedHex& msg) {
	notify(msg);

	alertEntitiesInPlayerFov();
}

void CHexWorld::onPlayerTurnEnd(CPlayerTurnEnd& msg){
	world.setTurnPhase(robotPhase);
	robotsChooseActions();
}

/** Return the highest priority object at this hex. ie, the one if 
	clicked-on we would want to respond. */
CGameHexObj* CHexWorld::getPrimaryObjectAt(CHex& hex) {
	TRange entities = map->getEntitiesAt(hex);

	for (auto it = entities.first; it != entities.second; it++) {
		if (it->second->isActor())
			return it->second;
	}

	for (auto it = entities.first; it != entities.second; it++) {
		if (it->second->isTigClass(tig::CItem))
			return it->second;
	}

	for (auto it = entities.first; it != entities.second; it++) {
		if (it->second->isTigClass(tig::CDoor))
			return it->second;
	}

	return &nullGameHexObj;
}

void CHexWorld::updateCameraPosition() {
	if (hexRenderer.following())
		hexRenderer.followTarget(playerObj->worldPos);
	else
		hexRenderer.attemptScreenScroll(/*mainApp->getMousePos()*/mousePos, dT);
}

void CHexWorld::beginNewTurn() {
	world.setTurnPhase(playerPhase);
	world.onscreenRobotAction = false;
	qps.beginNewTurn();
	playerObj->onTurnBegin();
	map->updateBlocking();
	hexRenderer.updateHexShaderBuffer();
	//chooseActions();
	cursorPath = map->aStarPath(playerObj->hexPosition, hexCursor->hexPosition);


	///////////////////////////temp end stuff
	if (playerObj->hexPosition == CHex(13,-4,-9)) {
		CSendText msg(combatLog, "\n\nYOU HAVE WON");
		send(msg);

		world.setTurnPhase(playerDeadPhase);
	}
}

void CHexWorld::endTurn() {
	qps.endTurn();
}

void CHexWorld::killEntity(CGameHexObj* entity) {
	unsubscribe(entity);

	auto simul = std::find(simulList.begin(), simulList.end(), entity);
	if (simul != simulList.end())
		simulList.erase(simul);

	auto serial = std::find(serialList.begin(), serialList.end(), entity);
	if (serial != serialList.end())
		serialList.erase(serial);

	auto drawable = std::find(entitiesToDraw.begin(), entitiesToDraw.end(), entity);
	if (drawable != entitiesToDraw.end())
		entitiesToDraw.erase(drawable);

	map->removeFromMap(entity);

	if (entity == playerObj)
		world.setTurnPhase(playerDeadPhase);

	delete entity;
}

/** Update which entities are now visible to the player. */
void CHexWorld::alertEntitiesInPlayerFov() {
	for (auto& entity : map->entities) {
		if (entity == playerObj)
			continue;
		bool inView = playerObj->viewField.searchView(entity->hexPosition);
		entity->playerSight(inView);
	}
}


