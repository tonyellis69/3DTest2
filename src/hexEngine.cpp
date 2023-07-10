#include "hexEngine.h"

#include <string>
#include <algorithm>

#include <glm/gtx/vector_angle.hpp> 

#include "UI\gui.h"

#include "utils/log.h"
#include "gameTextWin.h"
#include "gameState.h"
//#include "hexRenderer.h"

#include "sound/sound.h"

#include "UI/uiRender.h" //temp debug

#include "messaging/msg2.h" //for test

#include "gameGui.h"
 
#include "UI/fonts.h" //temp test

#include "renderer/imRendr/imRendr.h"

#include "GLFW/glfw3.h"

#include "hexRender/drawFunc.h"

#include "hexRender/destGraphic.h"
#include "hexRender/avoidGraphic.h"

#include "UI/gui2.h"

#include "win/win.h"

#include "UI/guiEvent.h"
#include "gameEvent.h"

#include "items/shield.h"

#include  "importer/importer.h"

#include "modules/workingMode.h" //remove
#include "modules/gameMode.h"
#include "modules/procGenMode.h"

#include "roboState.h"

CHexEngine::CHexEngine() {
	gameWorld.paused = true;

	hexRender.onSpawn();

	imRendr::setMatrix(&hexRender.camera.clipMatrix);
	
	lis::subscribe<CGUIevent>(this);
	lis::subscribe<CGameEvent>(this);
	lis::subscribe<CEngineEvent>(this);

	lis::subscribe<CGameEvent>(&hexRender);
	lis::subscribe<CGameEvent>(&physics);
	lis::subscribe<CPhysicsEvent>(&physics);

	hexPosLbl = gui::addLabel("xxx", 10, 10);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	snd::setVolume(1);

	gWin::initWindows();

	initPalettes();



}

void CHexEngine::initialise() {
	reticule = gameWorld.models["reticule"];
	reticule.palette[0] = { 1,1,1,1 };

	//gameMode = std::make_unique<CGameMode>(this);
	//mode = gameMode.get();
	procGenMode = std::make_unique<CProcGenMode>(this);
	mode = procGenMode.get();

	mode->initalise();


	gameWorld.paused = false;


}

void CHexEngine::onEvent(CGUIevent& e) {
	mode->guiHandler(e);

	//!!!Keep for now, not replicated in module
	if (e.type == eKeyDown) {
		if (e.i1 == GLFW_KEY_F2)
			toggleDirectionGraphics();
		else if (e.i1 == 'F')
			gameWorld.slowed = !gameWorld.slowed;
		else if (e.i1 == 'G')
			gameWorld.speeded = !gameWorld.speeded;
		else if (e.i1 == GLFW_KEY_F3)
			hexRender.tmpX = hexRender.tmpX == 0 ? 1 : 0;

		else if (e.i1 == '-')
			hexRender.tmpLineThickness -= 0.25f;
		else if (e.i1 == '=')
			hexRender.tmpLineThickness += 0.25f;
		else if (e.i1 == '[')
			hexRender.tmpLineSmooth -= 0.05f;
		else if (e.i1 == ']')
			hexRender.tmpLineSmooth += 0.05f;
		else if (e.i1 == 'O')
			hexRender.tmpLineSolid -= 0.05f;
		else if (e.i1 == 'P')
			hexRender.tmpLineSolid += 0.05f;

	/*	else if (e.i1 == 'L')
			hexRender.tmpKernel -= 2;*/
		else if (e.i1 == ';')
			hexRender.tmpKernel += 2;
		else if (e.i1 == '\'')
			hexRender.tmpSigma -= 0.1f;
		else if (e.i1 == '\\')
			hexRender.tmpSigma += 0.1f;
		else if (e.i1 == ',')
			hexRender.tmpBlurs -= 1;
		else if (e.i1 == '.')
			hexRender.tmpBlurs += 1;
		else if (e.i1 == '/')
		{
			hexRender.tmpBlurTextDivisor++;
			if (hexRender.tmpBlurTextDivisor > 5)
				hexRender.tmpBlurTextDivisor = 1;
			hexRender.resizeBlurTextures();
		}
		else if (e.i1 == 'N')
			hexRender.tmpFade -= 0.05f;
		else if (e.i1 == 'M')
			hexRender.tmpFade += 0.05f;

		else if (e.i1 == 'R')
			hexRender.recompileShader();
		else if (e.i1 == 'I') {
			switchModes();
		}


	}



	if (e.type == eMouseMove) {
		if (gameWorld.paused)
			return;
		
		lastMousePos = mousePos;
		mousePos = e.pos;
		calcMouseWorldPos();
	}


	if (e.type == eMouseWheel) {

		if (CWin::keyPressed(GLFW_KEY_LEFT_SHIFT)) {
			hexRender.pitchCamera(e.dy);
		}
		else {
			if (CWin::keyPressed(GLFW_KEY_LEFT_CONTROL)) {
				if (hexRender.dollyCamera(e.dy * zoomScale))
					adjustZoomScale(e.dy);
			}
			else {
				;
			}
		}
	}


}

void CHexEngine::onEvent(CGameEvent& e) {
	mode->gameEventHandler(e);
	switch (e.type) {
	case gamePlayerDeath:	onPlayerDeath(); break;
	case gameLevelChange: break;
	case gamePlayerSpawn: { if (cameraMode == camFollow)
		pFollowCamEnt = e.entity;
		gameWorld.player = e.entity;
	}; break;

	}
}

void CHexEngine::onEvent(CEngineEvent& e) {
	 if (e.action == engGetRenderer) {
		e.pHexRender = &hexRender;
	}
}



/**	Load a multipart mesh for storage under the given name. */
void CHexEngine::addMesh(const std::string& name, const std::string& fileName) {
	CImporter importer;
	importer.loadFile(fileName);	
	gameWorld.models[name] = importer.getModel();
}

/** Import a mesh to use in drawing the level. */
void CHexEngine::addHexTile(const std::string& name, const std::string& fileName, std::vector<glm::vec4>& colours) {
	CImporter importer;
	importer.loadFile(fileName);

	hexRender.addHexTile(name, importer.getVertData(),colours);

}


void CHexEngine::deleteMap() {
	//delete level;
}



void CHexEngine::startProcTest() {
	mode->startProcTest();
}


void CHexEngine::moveCamera(glm::vec3& direction) {
	float camSpeed =  10 * dT;
	glm::vec3 vector = direction * camSpeed;
	//hexRendr2.moveCamera(vector); //FIX: phase out!
	freeCamPos.x += vector.x; freeCamPos.y += vector.y;
}




void CHexEngine::calcMouseWorldPos() {
	glm::vec3 mouseWS = hexRender.screenToWS(mousePos.x, mousePos.y);
	mouseHex = worldSpaceToHex(mouseWS);
	lastMouseWorldPos = mouseWorldPos;
	mouseWorldPos = mouseWS;
	if (gameWorld.player) {
		glm::vec3 mouseVec = mouseWorldPos - gameWorld.player->getPos();
		gameWorld.player->playerC->setMouseDir(glm::normalize(mouseVec));
	} //needed for game.player orientation etc

	if (mouseHex != lastMouseHex) {
		//CMouseExitHex msg;
		//msg.leavingHex = hexCursor->transform->hexPosition;
		lastMouseHex = mouseHex;
		onNewMouseHex();
	}
}


void CHexEngine::draw() {
	hexRender.drawMap();

	hexRender.resetDrawLists();

	for (auto& entity : gameWorld.entities) {
		if (entity->live && entity->modelCmp && entity->modelCmp->drawFn->visible) 
			entity->modelCmp->draw(hexRender);
	}
	if (directionGraphicsOn) {
		for (auto& graphic : hexRender.graphics)
			graphic->draw(hexRender);
	}


	//hexRender.drawLineListDBG();

	hexRender.makeGlowShapes();

	hexRender.blur();

	hexRender.drawGlow();


	//draw masks first
	hexRender.drawMaskList(); 

	hexRender.drawLineList(); 

	hexRender.drawSolidList();
	hexRender.drawUpperLineList();

	hexRender.drawExplosionList();

	if (gameWorld.player && !gameWorld.player->playerC->dead) {
		//imRendr::setDrawColour({ 1.0f, 1.0f, 1.0f, 1.0f });
		//imRendr::drawLine(game.player->worldPos, mouseWorldPos);
		//hexCursor->draw();
		drawReticule();
	}

	if (gameWorld.player) {
		int sh = ((CShieldComponent*)gameWorld.player->playerC->shield->item)->hp;

		imRendr::drawText(600, 50, "HP: " + std::to_string(gameWorld.player->healthC->hp)
			+ " Shield: " + std::to_string(sh)

		);
	}

	//imRendr::drawText(300, 70, "thickness: " + std::to_string(hexRender.tmpLineThickness)
	//	+ " smoothing: " + std::to_string(hexRender.tmpLineSmooth));
	//imRendr::drawText(300, 90, "kernel: " + std::to_string(hexRender.tmpKernel)
	//	+ " sigma: " + std::to_string(hexRender.tmpSigma) + " blurs: "
	//	+ std::to_string(hexRender.tmpBlurs)
	//	+ " blur div: " + std::to_string(hexRender.tmpBlurTextDivisor)
	//	+ " solidity: " + std::to_string(hexRender.tmpLineSolid));

	if (editMode) {
		imRendr::drawText(600, 80, "Entity: " +  mapEdit.currentEntStr + " shape: " + mapEdit.currentShapeStr +
		mapEdit.entIdStr);

	}

	if (pBotZero && pBotZero->deleteMe == false)
		imRendr::drawText(600, 80, pBotZero->diagnostic);

}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexEngine::setAspectRatio(glm::vec2& ratio) {
	float defFov = glm::radians(45.0f);
	float defaultScreenH = 800.0f;
	float newFov = asin(sin(defFov / 2.0f) * ratio.y / defaultScreenH) * 2.0f;
	hexRender.setCameraAspectRatio(ratio,newFov);
	hexRender.setScreenSize(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexEngine::update(float dt) {
	this->dT = dt;

	mode->update(dT);

	calcMouseWorldPos();

	for (int n = 0; n < gameWorld.entities.size(); n++) {
		auto& entity = gameWorld.entities[n];
		if (entity->live)
			gameWorld.entities[n]->update(dT);
	}

	gameWorld.update(dT);

	physics.update(dT);

	gWin::update(dT);

}


void CHexEngine::toggleView() {
	if (viewMode == gameView)
		setViewMode(devView);
	else
		setViewMode(gameView);
}

void CHexEngine::toggleEditMode() {
	editMode = !editMode;
	if (editMode) {
		mapEdit.setMap(&gameWorld.level);
		freeCam();
		gWin::pNear->hideWin();
		CWin::showMouse(true);
	}
	else {
		followCam(gameWorld.player);
		gWin::pNear->showWin();
		prepMapEntities();
		CWin::showMouse(false);
	}
	
}

void CHexEngine::onUndo() {
	if (editMode)
		mapEdit.onUndo();
}

void CHexEngine::onRedo() {
	if (editMode)
		mapEdit.onRedo();
}


/** Plug the map's entities into physics and whatever else they need
	to be connected to. */
void CHexEngine::prepMapEntities() {
	physics.clearEntities();

	for (auto& entity : gameWorld.entities) {
		if (entity->isRobot)
			physics.add(entity.get());

		if (entity->entityType == entPlayer) {
			gameWorld.player = entity.get();
//			game.player = (Cgame.playerect*)entity.get();
			physics.add(entity.get());
		}


	}

}




/////////////public - private devide

//void CHexWorld::createCursorObject() {
//	hexCursor = new CEntity();
//
//	//hexCursor->setModel(spawn::models["cursor"]);
//	hexCursor->setPosition(glm::vec3(0, 0, 0));
//}


/** Respond to mouse cursor moving to a new hex. */
void CHexEngine::onNewMouseHex() {
	//hexCursor->setPosition(cubeToWorldSpace(mouseHex));


	/*if (!lineOfSight)
		cursorPath = level->aStarPath(game.player->hexPosition, hexCursor->hexPosition, true);
	else
		cursorPath = *hexLine2(game.player->hexPosition, hexCursor->hexPosition);*/

	std::stringstream coords; coords << "cube " << mouseHex.q << ", " << mouseHex.r << ", " << mouseHex.s;
	glm::i32vec2 offset = cubeToOffset(mouseHex);
	coords << "  offset " << offset.x << ", " << offset.y;
	glm::i32vec2 index = gameWorld.level.getHexArray()->cubeToIndex(mouseHex);
	coords << " index " << index.x << " " << index.y;
	glm::vec3 worldSpace = cubeToWorldSpace(mouseHex);
	coords << " worldPos " << worldSpace.x << " " << worldSpace.y << " " << worldSpace.z;
	coords << " " << gameWorld.level.getHexArray()->getHexCube(mouseHex).content;
	//glm::vec2 screenPos = hexRenderer.worldPosToScreen(worldSpace);
	coords << " wsMouse " << mouseWorldPos.x << " " << mouseWorldPos.y;
	coords << " scrnMouse " << mousePos.x << " " << mousePos.y;
//	CHex test = worldSpaceToHex3(glm::vec3(mouseWorldPos.x, mouseWorldPos.y,0));
//	coords << " rect " << test.x << " " << test.z;


	hexPosLbl->setText(coords.str());

	//lblText = coords.str();

	if (editMode)
		mapEdit.onNewMouseHex(mouseHex);

}





/** Handle an 'external function call' from Tig. */
int CHexEngine::tigCall(int memberId) {
	//std::string strParam;
	//switch (memberId) {
	//case tig::msgFn : 
	//	strParam = vm->getParamStr(0);
	//	liveLog << strParam; break;
	//case tig::popupWin :
	//	strParam = vm->getParamStr(0);
	//	;// mothballing this, use messaging popupMsg(strParam); break;
	//}
	return 1;
}





/** Whether we're in standard follow-cam mode or not, etc. */
void CHexEngine::setViewMode(TViewMode vMode) {
	viewMode = vMode;

	if (viewMode == gameView) {
		hexRender.pointCamera(glm::vec3(0, 0, -1));
		hexRender.setCameraHeight(15);
		followCam(gameWorld.player);
	}
	else if (viewMode == devView) {
		hexRender.setCameraPos(glm::vec3(0, -0, 12));
		hexRender.setCameraPitch(45);
	}
	else if (viewMode == keepView) {
		zoom2fit = true;
		freeCam();
	}

}

void CHexEngine::adjustZoomScale(float delta) {
	zoomAdjust += (delta > 0) ? -0.1f : 0.1f;
	zoomAdjust = std::max<float>(zoomAdjust, 0.0f);
	zoomScale = 1.0f + (std::pow(zoomAdjust, 0.5f) * 10);
}



void CHexEngine::toggleDirectionGraphics() {
	directionGraphicsOn = !directionGraphicsOn;

	if (directionGraphicsOn) {
		for (auto& entity : gameWorld.entities) {
			if (entity->isRobot) {
				auto graphic = std::make_shared<CAvoidGraphic>();
				graphic->entity = entity;
				graphic->pPalette = hexRender.getPalette("mix");
				hexRender.graphics.emplace_back(graphic);

				auto graphic2 = std::make_shared<CDestinationGraphic>();
				graphic2->entity = entity;
				graphic2->pPalette = hexRender.getPalette("blue");
				hexRender.graphics.emplace_back(graphic2);
			}
		}
	}
	else {
		hexRender.graphics.clear();
	}

}

void CHexEngine::followCam(CEntity* ent) {
	if (ent) {
		pFollowCamEnt = ent;
		cameraMode = camFollow;
	}
}

void CHexEngine::freeCam(float x, float y) {
	cameraMode = camFree;
	freeCamPos = glm::vec2(x, y);
}

void CHexEngine::freeCam() {
	cameraMode = camFree;
	freeCamPos = glm::vec2(hexRender.camera.getPos().x, hexRender.camera.getPos().y);
}

void CHexEngine::fixedCam(float x, float y) {
	cameraMode = camFixed;
	hexRender.setCameraPos(x, y);
}



void CHexEngine::drawReticule() {
	hexRender.drawModelAt(reticule, mouseWorldPos);	
}



///** Remove marked entities from the game. */
//void CHexWorld::removeEntities() {
//	physics.removeDeadEntities();
//
//	for (auto& gra = hexRender.graphics.begin(); gra != hexRender.graphics.end();) {
//		if (gra->get()->entity && gra->get()->entity.get()->toRemove == true)
//			gra = hexRender.graphics.erase(gra);
//		else
//			gra++;
//	}
//
//	for (auto& ent = game.entities.begin(); ent != game.entities.end();) {
//		if (ent->get()->toRemove == true) {
//			if (pBotZero && pBotZero->id == ent->get()->id) {
//				pBotZero = nullptr;
//			}
//			ent = game.entities.erase(ent);
//		}
//		else
//			ent++;
//	}
//
//}

void CHexEngine::onPlayerDeath() {
	fixedCam(gameWorld.player->getPos().x, gameWorld.player->getPos().y);
	//level->removeEntity(game.player);
	gameWorld.player->setPosition(glm::vec3(0));

	for (auto& entity : gameWorld.entities) {
		if (entity->isRobot) {
			//((CRobot*)entity.get())->setState(robotWander3);
			entity->addComponent<CRoboWander>();
		}
	}
}



/** TO DO: ultimately this should be automated via a Tig
	config file rather than hardcoded. */
void CHexEngine::initPalettes() {
	hexRender.storePalette("hex", { {0.0f,0.3f,0.0f,1.0f} });
	hexRender.storePalette("largeHex", { { 0.0f,0.0f,0.9f,1.0f}, { 0.5f,0.5f,0.8f,1.0f} });
	hexRender.storePalette("basic", { {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,1,1} });
	hexRender.storePalette("gun", { {0, 1, 1.0, 0.45f}, {0,0,0,1}, {1,1,1,1} });
	hexRender.storePalette("armour", { {1, 0, 1.0, 0.45f}, {0,0,0,1}, {1,1,1,1} });
	hexRender.storePalette("explosion", { {1, 1, 0.0,1}, {0,0,1,1}, {1,1,1,1} });
	hexRender.storePalette("mix", { {1,1,1,1}, {1,1,0,1}, {1,0,1,1}, {0,0,1,1} });
	hexRender.storePalette("blue", { {0,0,1,1}, {0.2,0.2,1,1}, {0.3,0.3,1,1}, {0,0,1,1} });
	hexRender.storePalette("test", { {1,0,0,1}, {0,1,0,1}, {0,0,1,1}, {1,1,0,1} });
	hexRender.storePalette("shield", { {0,0,1,0.15f}, {1,0,0,0.15f}, {0,1,0,1}, {1,1,0,1} });


	gameWorld.pPalettes = &hexRender.palettes;
}

void CHexEngine::switchModes() {
	if (mode == procGenMode.get()) {
		mode->onSwitchFrom();
		if (!gameMode) {
			gameMode = std::make_unique<CGameMode>(this);
			gameMode->initalise();
		}
		mode = gameMode.get();
		mode->onSwitchTo();
	}
	else {
		mode->onSwitchFrom();

		mode = procGenMode.get();
		mode->onSwitchTo();
	}
}