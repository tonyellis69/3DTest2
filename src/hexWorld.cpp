#include "hexWorld.h"

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

#include "spawner.h"

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

CHexWorld::CHexWorld() {
	game.paused = true;

	hexRender.init();

	imRendr::setMatrix(&hexRender.camera.clipMatrix);
	
	lis::subscribe<CGUIevent>(this);
	lis::subscribe<CGameEvent>(this);
	lis::subscribe<CPhysicsEvent>(this);

	hexPosLbl = gui::addLabel("xxx", 10, 10);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	snd::setVolume(1);

	gWin::initWindows();

	initPalettes();

	reticule = spawn::models["reticule"];
	reticule.palette[0] = { 1,1,1,1 };


}

void CHexWorld::onEvent(CGUIevent& e) {
	mode->guiHandler(e);

	//!!!Keep for now, not replicated in module
	if (e.type == eKeyDown) {
		if (e.i1 == GLFW_KEY_F2)
			toggleDirectionGraphics();
		else if (e.i1 == 'F')
			game.slowed = !game.slowed;
		else if (e.i1 == 'G')
			game.speeded = !game.speeded;
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

		else if (e.i1 == 'L')
			hexRender.tmpKernel -= 2;
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

	}


	if (e.type == eMouseMove) {
		if (game.paused)
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





	if (e.key == GLFW_KEY_ENTER && e.type == eKeyDown) {

		start();
	}



}

void CHexWorld::onEvent(CGameEvent& e) {
	mode->gameEventHandler(e);
	if (e.type == gamePlayerDeath) {
		onPlayerDeath();
	}
	else if (e.type == gameLevelChange) {
		physics.clearEntities();
		physics.setMap(game.level->getHexArray());
		hexRender.loadMap(game.level->getHexArray());
		prepMapEntities();
		setViewMode(mode->viewMode);
	}
}

void CHexWorld::onEvent(CPhysicsEvent& e) {
	if (e.action == physAdd)
		physics.add(e.entity);
	else if (e.action == physRemove)
		physics.remove(e.entity);
}


/**	Load a multipart mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, const std::string& fileName) {
	CImporter importer;
	importer.loadFile(fileName);	
	spawn::models[name] = importer.getModel();
}

/** Import a mesh to use in drawing the level. */
void CHexWorld::addHexTile(const std::string& name, const std::string& fileName, std::vector<glm::vec4>& colours) {
	CImporter importer;
	importer.loadFile(fileName);

	hexRender.addHexTile(name, importer.getVertData(),colours);

}


void CHexWorld::deleteMap() {
	//delete level;
}


/** Start a new session. */
//FIXME: is this for restarts too? 
void CHexWorld::start() {




	//default: game mode
	if (!mode) {
		//gameMode = std::make_unique<CGameMode>(this);
		//mode = gameMode.get();
		procGenMode = std::make_unique<CProcGenMode>(this);
		mode = procGenMode.get();
	}
	mode->start();

	//setViewMode(mode->viewMode); 


	game.paused = false;

	pBotZero = nullptr;
	for (auto& entity : game.entities) {
		if (entity->isRobot && entity->id == 7) {
			if (pBotZero == NULL)
				pBotZero = entity.get();
			else
				;// ((CRobot*)entity.get())->setState(robotDoNothing);
		}
	}


}

void CHexWorld::startProcTest() {
	mode->startProcTest();
}


void CHexWorld::moveCamera(glm::vec3& direction) {
	float camSpeed =  10 * dT;
	glm::vec3 vector = direction * camSpeed;
	//hexRendr2.moveCamera(vector); //FIX: phase out!
	freeCamPos.x += vector.x; freeCamPos.y += vector.y;
}




void CHexWorld::calcMouseWorldPos() {
	glm::vec3 mouseWS = hexRender.screenToWS(mousePos.x, mousePos.y);
	mouseHex = worldSpaceToHex(mouseWS);
	lastMouseWorldPos = mouseWorldPos;
	mouseWorldPos = mouseWS;
	if (playerObj) {
		glm::vec3 mouseVec = mouseWorldPos - playerObj->getPos();
		playerObj->setMouseDir(glm::normalize(mouseVec));
	} //needed for playerObj orientation etc

	if (mouseHex != lastMouseHex) {
		//CMouseExitHex msg;
		//msg.leavingHex = hexCursor->transform->hexPosition;
		lastMouseHex = mouseHex;
		onNewMouseHex();
	}
}


void CHexWorld::draw() {
	hexRender.drawMap();

	hexRender.resetDrawLists();

	for (auto& entity : game.entities) {
		if (entity->live) 
			//entity->drawFn.get()->draw(hexRender);
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

	if (game.player && !game.player->dead) {
		//imRendr::setDrawColour({ 1.0f, 1.0f, 1.0f, 1.0f });
		//imRendr::drawLine(playerObj->worldPos, mouseWorldPos);
		//hexCursor->draw();
		drawReticule();
	}

	if (game.player) {
		int sh = ((CShieldComponent*)game.player->shield->item.get())->hp;

		imRendr::drawText(600, 50, "HP: " + std::to_string(game.player->hp)
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
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	float defFov = glm::radians(45.0f);
	float defaultScreenH = 800.0f;
	float newFov = asin(sin(defFov / 2.0f) * ratio.y / defaultScreenH) * 2.0f;
	hexRender.setCameraAspectRatio(ratio,newFov);
	hexRender.setScreenSize(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dt) {

	mode->update(dT);

	this->dT = dt;

	updateCameraPosition();

	calcMouseWorldPos();

	physics.update(dT);


	if (game.entitiesToKill) {
		removeDeadEntities();
	}


	game.update(dT);

	if (game.level->mapUpdated) {
		//hexRendr2.setMap(level->getHexArray()); //temp to refresh map
		hexRender.loadMap(game.level->getHexArray());
		game.level->mapUpdated = false;
	}

	gWin::update(dT);

	for (auto& graphic : hexRender.graphics) {
		graphic->update(dT);
	}




}


void CHexWorld::toggleView() {
	if (viewMode == gameView)
		setViewMode(devView);
	else
		setViewMode(gameView);
}

void CHexWorld::toggleEditMode() {
	editMode = !editMode;
	if (editMode) {
		mapEdit.setMap(game.level.get());
		freeCam();
		gWin::pNear->hideWin();
		CWin::showMouse(true);
	}
	else {
		followCam(playerObj);
		gWin::pNear->showWin();
		prepMapEntities();
		CWin::showMouse(false);
	}
	
}

void CHexWorld::onUndo() {
	if (editMode)
		mapEdit.onUndo();
}

void CHexWorld::onRedo() {
	if (editMode)
		mapEdit.onRedo();
}


/** Plug the map's entities into physics and whatever else they need
	to be connected to. */
void CHexWorld::prepMapEntities() {
	physics.clearEntities();

	for (auto& entity : game.entities) {
		if (entity->isRobot)
			physics.add(entity.get());

		if (entity->entityType == entPlayer) {
			game.player = (CPlayerObject*)entity.get();
			playerObj = (CPlayerObject*)entity.get();
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
void CHexWorld::onNewMouseHex() {
	//hexCursor->setPosition(cubeToWorldSpace(mouseHex));


	/*if (!lineOfSight)
		cursorPath = level->aStarPath(playerObj->hexPosition, hexCursor->hexPosition, true);
	else
		cursorPath = *hexLine2(playerObj->hexPosition, hexCursor->hexPosition);*/


	std::stringstream coords; coords << "cube " << mouseHex.x << ", " << mouseHex.y << ", " << mouseHex.z;
	glm::i32vec2 offset = cubeToOffset(mouseHex);
	coords << "  offset " << offset.x << ", " << offset.y;
	glm::i32vec2 index = game.level->getHexArray()->cubeToIndex(mouseHex);
	coords << " index " << index.x << " " << index.y;
	glm::vec3 worldSpace = cubeToWorldSpace(mouseHex);
	coords << " worldPos " << worldSpace.x << " " << worldSpace.y << " " << worldSpace.z;
	coords << " " << game.level->getHexArray()->getHexCube(mouseHex).content;
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
int CHexWorld::tigCall(int memberId) {
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


void CHexWorld::updateCameraPosition() {	
	if (cameraMode == camFollow) {
		if (pFollowCamEnt) {
			 hexRender.setCameraPos(pFollowCamEnt->getPos().x, pFollowCamEnt->getPos().y);
		}
	}
	else if (cameraMode == camFree) {
		hexRender.setCameraPos(freeCamPos.x, freeCamPos.y);
	}

	if (zoom2fit)
		zoomToFit();

}



/** Whether we're in standard follow-cam mode or not, etc. */
void CHexWorld::setViewMode(TViewMode vMode) {
	viewMode = vMode;

	if (viewMode == gameView) {
		hexRender.pointCamera(glm::vec3(0, 0, -1));
		hexRender.setCameraHeight(15);
		followCam(game.player);
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

void CHexWorld::adjustZoomScale(float delta) {
	zoomAdjust += (delta > 0) ? -0.1f : 0.1f;
	zoomAdjust = std::max<float>(zoomAdjust, 0.0f);
	zoomScale = 1.0f + (std::pow(zoomAdjust, 0.5f) * 10);
}



void CHexWorld::toggleDirectionGraphics() {
	directionGraphicsOn = !directionGraphicsOn;

	if (directionGraphicsOn) {
		for (auto& entity : game.entities) {
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

void CHexWorld::followCam(CEntity* ent) {
	if (ent) {
		pFollowCamEnt = ent;
		cameraMode = camFollow;
	}
}

void CHexWorld::freeCam(float x, float y) {
	cameraMode = camFree;
	freeCamPos = glm::vec2(x, y);
}

void CHexWorld::freeCam() {
	cameraMode = camFree;
	freeCamPos = glm::vec2(hexRender.camera.getPos().x, hexRender.camera.getPos().y);
}

void CHexWorld::fixedCam(float x, float y) {
	cameraMode = camFixed;
	hexRender.setCameraPos(x, y);
}



void CHexWorld::drawReticule() {
	hexRender.drawModelAt(reticule, mouseWorldPos);	
}

void CHexWorld::removeDeadEntities() {
	physics.removeDeadEntities();

	for (auto& ent = game.entities.begin(); ent != game.entities.end();) {
		if (ent->get()->deleteMe)
			ent = game.entities.erase(ent);
		else
			ent++;
	}


	game.entitiesToKill = false;
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

void CHexWorld::onPlayerDeath() {
	fixedCam(playerObj->getPos().x, playerObj->getPos().y);
	//level->removeEntity(playerObj);
	playerObj->setPosition(glm::vec3(0));

	for (auto& entity : game.entities) {
		if (entity->isRobot) {
			//((CRobot*)entity.get())->setState(robotWander3);
			entity->ai = std::make_shared<CRoboWander>((CRobot*)entity.get());
		}
	}
}

/** Progressively zoom while the map doesn't fit the screen. */
void CHexWorld::zoomToFit() {
	glm::vec3 gridTL = abs(cubeToWorldSpace(game.level->indexToCube( glm::i32vec2{ 0,0 })));
	glm::vec3 gridBR = abs(cubeToWorldSpace(game.level->indexToCube(game.level->getGridSize())));

	glm::i32vec2 scnSize = hexRender.getScreenSize();
	glm::vec3 scnTL = abs(hexRender.screenToWS(0, 0));
	glm::vec3 scnBR = abs(hexRender.screenToWS(scnSize.x, scnSize.y));

	glm::vec3 margin(2, 2, 0);

	if (glm::any(glm::greaterThan(gridTL + margin, scnTL))) {
		hexRender.dollyCamera(-4.0f);
	} 
	else	if (glm::all(glm::lessThanEqual(gridTL , scnTL - margin * 1.5f))) {
		hexRender.dollyCamera(1.0f);
	}
	else
		zoom2fit = false;

}

/** TO DO: ultimately this should be automated via a Tig
	config file rather than hardcoded. */
void CHexWorld::initPalettes() {
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


	spawn::pPalettes = &hexRender.palettes;
}