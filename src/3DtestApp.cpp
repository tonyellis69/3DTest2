#include "3DtestApp.h"

#include <glew.h>
#include <algorithm>
#include <string>
#include <vector>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/common.hpp>
#include <glm/gtx/rotate_vector.hpp> 

#include "colour.h"

#include <glm/gtc/matrix_access.hpp>	 //temp

#include "watch.h"

#include "UI\GUIimage.h"
#include "UI\GUIlabel2.h"
#include "UI\GUIcheckButton.h"




#include "plants\fractalTree.h"

using namespace watch;

using namespace glm;



C3DtestApp::C3DtestApp() {
	
	tmpSCno = 0;
	tmp = false;
	physCube = NULL;
	shownChoice = false;
}

void C3DtestApp::onStart() {
	
	

	chunkCall = 0;

	dataPath = homeDir + "Data\\";
	lastMousePos = glm::vec2(0, 0);

	//test objects, temporary
	cube = Engine.createCube(vec3(-3, 300, -3), vec3(1.0f)); Engine.modelDrawList.push_back(cube);
	Engine.modelDrawList.push_back(Engine.createCube(vec3(3, 300, -3), vec3(1.0f)));
	Engine.createCylinder(vec3(0, 300, -4), 1, 2, 30);


	//position the default camera
	
	Engine.getCurrentCamera()->setPos(vec3(-3, 300, 3));
	Engine.getCurrentCamera()->lookAt(vec3(0.0746933, -0.291096, 0.953797));

	//for centre/tree view
	Engine.getCurrentCamera()->setPos(vec3(0.499624, 6.46961, 21.5411));
	Engine.getCurrentCamera()->lookAt(vec3(0.0130739, -0.0538602, -0.998463));

	//pulled back view
	Engine.getCurrentCamera()->setPos(vec3(-1510.24, 2833.93, 3682.67));
	Engine.getCurrentCamera()->lookAt(vec3(0.31595, -0.609743, -0.726901));


	//Position FPS camera
	fpsCam.setPos(vec3(0, 180, 0));
	fpsCam.lookAt(vec3(0, -1, -3));
	fpsOn = false;
	selectChk = i32vec3(0, 0, 0);
	mouseLook = false;
	
		
	CTerrainPhysObj* terrainPhysObj = new CTerrainPhysObj();
	terrainPhysObj->attachModel(&terrain);
	terrainPhysObj->setCollides(false);
	Engine.physObjManager.addPhysObj(terrainPhysObj);
	
	//
	
	
	CBaseBuf* terrainBuf = &terrain.multiBuf; //TO DO: ugh, make a setter
	//((CMultiBuf*)terrainBuf)->setRenderer(&Engine.Renderer);
	
	terrainBuf->setSize(175000000);
	
	terrainBuf->storeLayout(3, 3, 0, 0);
	
	tempFeedbackBuf = Engine.createBuffer();
	tempFeedbackBuf->setSize(1000000);

	terrain.EXTfreeChunkModel.Set(&Engine, &CEngine::freeModel);
	//

	terrain.chunkDrawShader = Engine.Renderer.phongShader;

	terrain.setSizes(chunksPerSuperChunkEdge, cubesPerChunkEdge, cubeSize);
	//terrain->createLayers(5120, 320, 2); //1280 320
	//terrain->createLayers(1280, 320, 0);
	terrain.createLayers(10000, 320, 2);
	//terrain->createLayers(1280, 320, 0);
	//terrain->createLayers(8, 2, 2); //(8, 3, 2); //(4,2,1);

	terrain.initChunkShell();
	terrain.initChunkGrid(cubesPerChunkEdge);

	

	terrain.initHeightFinder();

	
	
	

	terrain.createTerrain(vec2(6, 6)); //seem to get striations above 999

	initHeightmapGUI();
	

	double t = Engine.Time.milliseconds();
	SCpassed = SCrejected = 0;

	terrain.createAllChunks();

	

	t = Engine.Time.milliseconds() - t;
	


	

	skyDome = Engine.createSkyDome();

	oldTime = Engine.Time.seconds();

	initWireSCs();
	supWire = false;

	//initialise player object
	playerObject.pModel = Engine.createCube(vec3(0), vec3(playerObject.width*1, playerObject.height, playerObject.width*1));
	playerObject.setPos(vec3(0, 237, 0));
	playerObject.setPos(vec3(0, 0, 0));

	playerPhys = Engine.addPhysics(&playerObject);
	playerPhys->setMass(10);
	playerPhys->AABB.setSize(1, 1);
	playerPhys->asleep = true;

	
	
	
	CFractalTree fractalTree;
	/*
	//standard tree:
	fractalTree.setStemLength(7.0f, 0.2f);
	fractalTree.setNumBranches(6,3);
	fractalTree.setBranchAngle(30, 0.2f);
	fractalTree.setStemRadius(0.6f);
	fractalTree.setMaxStages(4);
	fractalTree.setStemFaces(5);
	fractalTree.setStageScale(0.6f);
	fractalTree.setBranchType(split);
	*/

	/*
	//work in progress lateral branching tree
	fractalTree.setStemLength(7.0f, 0.2f);
	fractalTree.setNumBranches(42, 3);
	fractalTree.setBranchAngle(80, 0.1f);
	fractalTree.setStemRadius(0.3f);
	fractalTree.setMaxStages(2);
	fractalTree.setStageScale(0.5f);
	fractalTree.setStemFaces(5);
	fractalTree.setBranchType(lateral);
	*/

	fractalTree.setLength(10.0f, 0.2f);
	fractalTree.setMaxJoints(3);
	fractalTree.setJointAngle(5, 0);
	fractalTree.setNumBranches(4, 1);
	fractalTree.setBranchAngle(40, 0.2f);
	fractalTree.setStemRadius(0.5f);
	fractalTree.setMaxStages(4);
	fractalTree.setStemFaces(5);
	fractalTree.setStageScale(0.6f);
	fractalTree.setBranchType(split);
	fractalTree.setNumLateralPoints(2);
	fractalTree.setLeadingBranch(true);

	fractalTree.create();
	terrain.tree = Engine.createModel();
	fractalTree.getModel(terrain.tree);

	terrain.initTreeFinding();
	terrain.initGrassFinding();

	initTextWindow();

	//vm.loadProgFile(dataPath + "..\\..\\TC\\Debug\\output.tig");
	vm.loadProgFile(dataPath + "..\\..\\TC\\output.tig");


	CGUIlabel2* lbl = new CGUIlabel2(300, 290, 300, 80);
	lbl->setFont(&sysFont);
	lbl->setTextColour(UIwhite);
	lbl->setText("A line that runs on long enough to need wrapping, possibly onto a third line.");
	lbl->setMultiLine(true);
	lbl->borderOn(true);
	GUIroot.Add(lbl);



	return;
}






/** Called every frame, provides a place for the user to check input where constant feedback is required. */
void C3DtestApp::keyCheck() {
	CCamera* currentCamera = Engine.getCurrentCamera();
	float moveInc = float( dT * 1000.0); // 0.05125f;

	if (!fpsOn) {

		if (keyNow('E')) {
			moveInc *= 0.1f;
			currentCamera->dolly(moveInc);
		}

		if (keyNow('A')) {
			currentCamera->track(-moveInc);
		}

		if (keyNow('D')) {
			currentCamera->track(moveInc);
		}

		if (keyNow('W')) {
			currentCamera->dolly(moveInc);
		}

		if (keyNow('S')) {
			currentCamera->dolly(-moveInc);
		}
		if (keyNow(' ')) {
			 currentCamera->elevate(moveInc);
		}


		if (keyNow('E')) {
			currentCamera->yaw(float(yawAng * dT));
		}

		if (keyNow('T')) {
			currentCamera->yaw(float(-yawAng *dT));
		}

		float rot = dT * 200.0f;
		if (keyNow('P')) {
			cube->rotate(rot, glm::vec3(1, 0, 0));
		}
		if (keyNow('Y')) {
			cube->rotate(rot, glm::vec3(0, 1, 0));
		}
		

	}
	else {
		vec3 moveDir(0);
		if (length(playerPhys->currentContactNormal) <= 0) {
			cerr << "\nmove aborted";	
		}
		else {


			vec3 dir = playerObject.povCam.getTargetDir();
			vec3 groundNormal = playerPhys->currentContactNormal;
			vec3 eyeLine = playerObject.povCam.getTargetDir();
			vec3 flip;

			if (keyNow(' ') && physCube == NULL) {
				playerPhys->velocity.y += 20;
				playerPhys->velocity.x *= 2.5f;
				playerPhys->velocity.z *= 2.5f;

			}

			if (keyNow('W')) {
				moveDir = cross(eyeLine, groundNormal) / length(groundNormal);
				moveDir = cross(groundNormal, moveDir) / length(groundNormal);
				moveDir.y = 0; //*
				playerPhys->velocity += moveDir * 0.35f;// was 1.4f;   //0.03f safe but slow //0.2f formerly caused bounces

				//TO DO: 0.35 causes bounce on steep ascent when looking up. 0.25f does not. 
				//probably doesn't even need fixing as those aren't realistic conditions
				//*Setting y=0 fixed it... look into scrapping the whole velocity-parallel-to-the-ground thing

		

			}
			if (keyNow('S')) {
				flip.y = -eyeLine.y;
				flip.x = -eyeLine.x;
				flip.z = -eyeLine.z;
				moveDir = cross(flip, groundNormal) / length(groundNormal);
				moveDir = cross(groundNormal, moveDir) / length(groundNormal);
				playerPhys->velocity += moveDir * 0.4f;
			}
			if (keyNow('A')) {
				flip.x = eyeLine.z;
				flip.z = -eyeLine.x;
				moveDir = cross(flip, groundNormal) / length(groundNormal);
				moveDir = cross(groundNormal, moveDir) / length(groundNormal);
				playerPhys->velocity += moveDir * 0.4f;
			}
			if (keyNow('D')) {
				flip.x = -eyeLine.z;
				flip.z = eyeLine.x;
				moveDir = cross(flip, groundNormal) / length(groundNormal);
				moveDir = cross(groundNormal, moveDir) / length(groundNormal);
				playerPhys->velocity += moveDir * 0.4f;
			}

		}


	}

	
	
	if (mouseKey == MK_LBUTTON)
	{
		if (!mouseLook) {
			mouseLook = true;
			//mouse capture on
			mouseCaptured(true);
			oldMousePos = vec2(mouseX, mouseY);
			showMouse(false);
			//centre mouse
			//setMousePos(-1, -1);
			setMousePos(viewWidth / 2, viewHeight / 2);
		}
		else {
			//find mouse movement
			glm::vec2 mousePos((mouseX - (viewWidth / 2)), ((viewHeight / 2) - mouseY));
			if (mousePos.x == 0 && mousePos.y == 0)
				return;
			float angle = (0.1f * length(mousePos));

			//move camera
			vec3 perp = normalize(vec3(mousePos.y, -mousePos.x, 0));
			currentCamera->freeRotate(perp, angle);

			//setMousePos(-1, -1);
			setMousePos(viewWidth / 2, viewHeight / 2);
		}

	}
	else { //mouselook key not down so
		if (mouseLook) {
			mouseCaptured(false);
			mouseLook = false;
			setMousePos(oldMousePos.x, oldMousePos.y);
			showMouse(true);

		}

	}
	

	if (keyNow('8')) {
		advance(north);
		//	EatKeys();
	}
	if (keyNow('2')) {
		advance(south);
		//EatKeys();
	}
	if (keyNow('6')) {
		advance(east);
		//EatKeys();
	}
	if (keyNow('4')) {
		advance(west);
		//EatKeys();
	}
	if (keyNow('5')) {
		advance(up);
	}
	if (keyNow('0')) {
		advance(down);
	}





	selectChk = glm::mod(vec3(selectChk), vec3(15, 5, 15));

}

/** Triggered *when* a key is pressed, not while it is held down. This is not 'whileKeyDown'. */
void C3DtestApp::onKeyDown( int key, long mod) {
	if (key == '1' || key == '2' || key == '3') {
		int num = 0;
		if (key == '2')
			num = 1;
		if (key == '3')
			num = 2;
		shownChoice = false;
		TVMmsg msg;
		msg.type = vmMsgChoice;
		msg.integer = num;
		vm.sendMessage(msg);
	}


	if (key == 'R') {
		//cube->rotate(rot, glm::vec3(0, 0, 1));
		//chunkShader->recompile();
		terrain.grassShader->recompile();
		//EatKeys();
	}

	if (key == '1') {
		fpsOn = !fpsOn;
		if (fpsOn) {
			//Engine.setCurrentCamera(&fpsCam);
			Engine.setCurrentCamera(&playerObject.povCam);
			playerPhys->asleep = false;
		}
		else
			Engine.setCurrentCamera(Engine.defaultCamera);
		//EatKeys();

	}

	if (key == 'X') {
		/*	terrain->clear();
		chunkShader->recompile();
		terrain2texShader->recompile();
		terrain->createAllChunks();
		updateHeightmapImage();
		*/
		for (int s = 0; s < terrain.layers[3].superChunks.size(); s++) {
			CSuperChunk* sc = terrain.layers[3].superChunks[s];
			if (sc->tmpIndex == i32vec3(2, 1, 1)) {
				sc->removeAllChunks();

			}
		}
		//EatKeys();
	}

	if (key == 'C') {
		/*	vec3 pos = currentCamera->getPos();
		pos = pos + currentCamera->getTargetDir() * 200.0f;
		CModel* cube = Engine.createCube(pos, vec3(40));
		physCube = Engine.addPhysics(cube);
		physCube->setMass(10);
		physCube->bSphere.setRadius(35);
		physCube->AABB.setSize(40, 40);*/

		terrain.createAllChunks();

		//EatKeys();
	}

	if (key == 'B') {
		vec3 camPos = Engine.getCurrentCamera()->getPos();
		//camPos = physCube->getModel()->getPos();
		cerr << "\ncam pos " << camPos.x << " " << camPos.y << " " << camPos.z;
		vec3 camTarg = Engine.getCurrentCamera()->getTargetDir();
		cerr << "\ncam target " << camTarg.x << " " << camTarg.y << " " << camTarg.z;


		//EatKeys();
	}
	

	//if (keyNow('U') )
	if (key == 'U') {
		supWire = !supWire;
		//EatKeys();
	}

	if (key == 'H') {
		heightmapImage->setVisible(!heightmapImage->getVisible());
		if (heightmapImage->getVisible())
			updateHeightmapImage();
		//EatKeys();
	}



};


/** Called when mouse moves. */
void C3DtestApp::mouseMove(int x, int y, int key) {
}

/*
void C3DtestApp::onResize(int width, int height) {
}
*/

void C3DtestApp::draw() {

	mat4 fpsCam = playerObject.povCam.clipMatrix;// *terrain->chunkOrigin;
	terrain.updateVisibleSClist(fpsCam);

	double t = Engine.Time.milliseconds();

	mat4 mvp = Engine.getCurrentCamera()->clipMatrix * terrain.chunkOrigin;

	//draw chunks
	mat3 tmp;
	Engine.Renderer.setShader(Engine.Renderer.phongShader);
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hNormalModelToCameraMatrix,tmp); //why am I doing this?
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hMVP, mvp);
	terrain.drawVisibleChunks();/////////////////////////////
	
	//draw grass
	Engine.Renderer.setShader(terrain.grassShader);
	Engine.Renderer.attachTexture(0, *terrain.grassTex);
	terrain.grassShader->setTextureUnit(0, terrain.hGrassTexure);
	terrain.grassShader->setShaderValue(terrain.hGrassTime,(float)Time);
	terrain.grassShader->setShaderValue(terrain.hGrassMVP,mvp);
	//terrain.drawGrass(mvp, terrain.visibleSClist);


	//draw trees
	Engine.Renderer.setShader(Engine.Renderer.phongShaderInstanced);
	Engine.Renderer.phongShaderInstanced->setShaderValue(Engine.Renderer.hPhongInstNormalModelToCameraMatrix,tmp);
	Engine.Renderer.phongShaderInstanced->setShaderValue(Engine.Renderer.hPhongInstMVP,mvp);

	glEnable(GL_PRIMITIVE_RESTART);
	
	//terrain.drawTrees(mvp, terrain.visibleSClist);

	glDisable(GL_PRIMITIVE_RESTART);

	t = Engine.Time.milliseconds() - t;


	//wireframe drawing:
	//draw bounding boxes
	if (supWire) {
		int component = 1;
		CSuperChunk* sc;
		vec4 colour = vec4(0, 0, 0, 1);
		vec3 cornerAdjust, opCornerAdjust;
		Engine.Renderer.setShader(terrain.wireBoxShader);
		for (int l = terrain.layers.size()-1; l > -1; l--) {
			vBuf::T3DnormVert box[2000]; //should be enough
			int index = 0;
			for (int s = 0; s < terrain.layers[l].superChunks.size(); s++) {
				sc = terrain.layers[l].superChunks[s];
				if (sc->nonEmpty) 
				{
					box[index].v = sc->nwWorldPos + terrain.layers[l].nwLayerPos;
					cornerAdjust = vec3(sc->faceBoundary[west], sc->faceBoundary[down], sc->faceBoundary[north]);
					box[index].v += cornerAdjust * sc->chunkSize;

					opCornerAdjust = vec3(sc->faceBoundary[east] + 1, sc->faceBoundary[up] + 1,
						sc->faceBoundary[south] + 1);
					opCornerAdjust -= cornerAdjust;
					box[index].normal = opCornerAdjust * sc->chunkSize;
					index++;
				}
			}
			wireSCs->storeVertexes(box, sizeof(vBuf::T3DnormVert) , index);
			wireSCs->storeLayout(3, 3, 0, 0);
			mvp = Engine.getCurrentCamera()->clipMatrix * wireSCs->worldMatrix;
			terrain.wireBoxShader->setShaderValue(terrain.hBoxMVP,mvp);
			
			colour[component++] = 0.8f;
			if (component > 2)
				component = 0;
			terrain.wireBoxShader->setShaderValue(terrain.hBoxColour,colour);
			wireSCs->drawNew();
		}
		
	
	}

	if (!fpsOn) {
		Engine.Renderer.setShader(Engine.Renderer.phongShader);
		mvp = Engine.getCurrentCamera()->clipMatrix * playerObject.pModel->worldMatrix;
		Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hMVP,mvp);
		playerObject.pModel->drawNew();
	}

	///watch::watch1 << "text";
}



void C3DtestApp::advance(Tdirection direction) {
	//NB direction is the direction new terrain is scrolling *in* from
	//dir is the vector that moves the terrain *away* from that direction, to make space for new terrain 
	vec3 dir;
	switch (direction) {
	case north:	dir = vec3(0, 0, 1); //z=1 = out of the screen
		break;
	case south:	dir = vec3(0, 0, -1);
		break;
	case east:		dir = vec3(-1, 0, 0);
		break;
	case west:		dir = vec3(1, 0, 0);
		break;
	case up:		dir = vec3(0, -1, 0);
		break;
	case down:		dir = vec3(0, 1, 0);
		break;
	}

	//Move terrain in given direction
	vec3 movement = dir *  float(10.0f);  //was 1
	//terrain->translate(movement);
	//vec3 pos = terrain->getPos();
	//terrain->chunkOrigin[3] += vec4(movement, 0);

	terrain.scrollTriggerPoint += vec3(movement);
	vec3 pos = terrain.scrollTriggerPoint;


	int chunkDist = cubesPerChunkEdge * cubeSize; //span of a chunk in world space
	bvec3 outsideChunkBoundary = glm::greaterThan(glm::abs(pos), vec3(chunkDist, chunkDist, chunkDist));

	//If terrain has moved by the length of a chunk
	if (glm::any(outsideChunkBoundary)) {
		vec3 posMod;
		posMod = glm::mod(pos, vec3(chunkDist, chunkDist, chunkDist)); //glm::mod seems to turn negative remainders positive
		posMod.x = fmod(pos.x, chunkDist);
		posMod.y = fmod(pos.y, chunkDist);
		posMod.z = fmod(pos.z, chunkDist);
		//	terrain->setPos(posMod ); //secretly move terrain back before scrolling to ensure it scrolls on the spot
		//	terrain->chunkOrigin[3] = glm::vec4(posMod, 1);

		if (outsideChunkBoundary.x) {
			pos.x = 0;
		}
		if (outsideChunkBoundary.y) {
			pos.y = 0;
		}
		if (outsideChunkBoundary.z) {
			pos.z = 0;
		}
		terrain.chunkOriginInt += dir;

		terrain.scrollTriggerPoint = pos;
		vec3 translation =  vec3(terrain.chunkOriginInt *  cubesPerChunkEdge) * cubeSize ;
		terrain.chunkOrigin[3] = vec4(translation, 1);
		terrain.advance(direction); 
		onTerrainAdvance(direction);
	}
}

/** Called every frame. Mainly use this to scroll terrain if we're moving in first-person mode*/
void C3DtestApp::Update() {
	vmUpdate();

	if (skyDome)
		skyDome->update(dT);

	terrain.update();

	vec3 pos = Engine.getCurrentCamera()->getPos();


	
	if (fpsOn) {
		//return;
	//	if (terrain->toSkin.size() != 0)
	//		return;
		//cheap dirty fix for the problem of scrolling in one direction before we've finished scrolling in another

		Tdirection direction = none;

		float chunkDist = cubesPerChunkEdge * cubeSize;

		vec3 pos;
		pos = playerObject.getPos();
		bvec3 outsideChunkBoundary = glm::greaterThan(glm::abs(pos), vec3(chunkDist));


		//has viewpoint moved beyond the length of one chunk?
		if (outsideChunkBoundary.x || outsideChunkBoundary.y || outsideChunkBoundary.z) {
			vec3 posMod;
			posMod.x = fmod(pos.x, chunkDist);
			posMod.y = fmod(pos.y, chunkDist);  //was pos.y
			posMod.z = fmod(pos.z, chunkDist);

			
			//WAS
			//fpsCam.setPos(posMod); //secretly reposition viewpoint prior to scrolling terrain
			playerObject.setPos(posMod);
			playerPhys->position = posMod;
			;

			vec3 translation = vec3(terrain.chunkOriginInt *  cubesPerChunkEdge) * cubeSize;
			//work out direction to scroll-in new terrain from
			if (outsideChunkBoundary.x) {
				if (pos.x > 0)
					direction = east;
				else
					direction = west;
				terrain.chunkOriginInt += dirToVec(flipDir(direction));
				terrain.chunkOrigin[3] = vec4(chunkDist * vec3(terrain.chunkOriginInt), 1);
				terrain.advance(direction);
				//return;
			}

			if (outsideChunkBoundary.y) {
				if (pos.y > 0)
					direction = up;
				else
					direction = down;
				terrain.chunkOriginInt += dirToVec(flipDir(direction));
				terrain.chunkOrigin[3] = vec4(chunkDist * vec3(terrain.chunkOriginInt), 1);
				terrain.advance(direction);
				//return;
			}


			if (outsideChunkBoundary.z) {
				if (pos.z > 0)
					direction = south;
				else
					direction = north;
				terrain.chunkOriginInt += dirToVec(flipDir(direction));
				terrain.chunkOrigin[3] = vec4(chunkDist * vec3(terrain.chunkOriginInt) , 1);
				terrain.advance(direction);
			}
		}
	}
}



/** Create a model for drawing superChunk positions in wireframe. */
void C3DtestApp::initWireSCs() {
	wireSCs = Engine.createModel();
	
	wireSCs->drawMode = GL_POINTS;
	wireSCs->getMaterial()->setShader(terrain.wireBoxShader);

	wireSCs->setPos(vec3(0));


	
}

/**	Called when terrain advances - ie, moves. */
void C3DtestApp::onTerrainAdvance(Tdirection direction) {
	if (physCube) {
	//	physCube->pModel->translate(-dirToVec(direction) * (float)cubesPerChunkEdge * cubeSize);
		//physCube->position += -dirToVec(direction) * (float)cubesPerChunkEdge * cubeSize;
	}
	//playerObject.translate(-dirToVec(direction) * (float)cubesPerChunkEdge * cubeSize);
}

/** Create the GUI and textures for displaying 2D terrain heightmaps. */
void C3DtestApp::initHeightmapGUI() {
	heightmapImage = new CGUIimage(10, 10, 500, 500);
	GUIroot.Add(heightmapImage);
	heightmapTex = Engine.Renderer.textureManager.createEmptyTexture(500, 500);
	heightmapImage->setTexture(*heightmapTex);
	
	//heightmapImage->visible = false;
	heightmapImage->setVisible(false);

	terrain2texShader = Engine.Renderer.createShader(dataPath + "terrain2tex");
	terrain2texShader->setType(userShader);
	Engine.Renderer.setShader(terrain2texShader);
	hTer2TexNwSampleCorner = terrain2texShader->getUniformHandle("nwSampleCorner");
	hTer2TexPixelScale = terrain2texShader->getUniformHandle("pixelScale");
	

}

/** Render the current terrain shader to our heightmap image. */
void C3DtestApp::updateHeightmapImage() {
	//activate the render-to-texture shader

	heightmapImage->setTexture(*heightmapTex);


	heightmapImage->setTexture(*heightmapTex);
	Engine.Renderer.setShader(terrain2texShader);
	vec3 sampleCorner = terrain.layers[0].nwSampleCorner;
	//terrain2texShader->setNwSampleCorner(vec2(sampleCorner.x,sampleCorner.z));
	terrain2texShader->setShaderValue(hTer2TexNwSampleCorner, vec2(sampleCorner.x, sampleCorner.z));

	//calculate scaling
	float scale = (terrain.worldSize.x / terrain.worldUnitsPerSampleUnit) / heightmapTex->width;

	//terrain2texShader->setPixelScale(scale);
	terrain2texShader->setShaderValue(hTer2TexPixelScale, scale);

	Engine.Renderer.renderToTextureQuad(*heightmapTex);

	
}

void C3DtestApp::initTextWindow() {
	CGUIpanel* backPanel = new CGUIpanel(200, 50, 800, 175);
	UIcolour tint = { 0,0,0,0.2f };
	backPanel->setBackColour1(tint);
	backPanel->setBackColour2(tint);
	backPanel->borderOn(false);
	GUIroot.Add(backPanel);

	textWindow = new CGUIrichText(10, 10, 780, 155);
	textWindow->setFont(&sysFont);
	textWindow->setTextColour(UIwhite);
	textWindow->hFormat = hCentre;
	textWindow->borderOn(false);
	textWindow->setMultiLine(true);
	backPanel->Add(textWindow);

	choiceMenu = new CGUImenu(300, 400, 500, 200);
	GUIroot.Add(choiceMenu);
	choiceMenu->setFont(&sysFont);
	choiceMenu->setTextColour(UIwhite);
	/////////////////////GUIroot.setFocus(choiceMenu);
	menuID = choiceMenu->getID();
	choiceMenu->setVisible(false);
}

/** Handle messages from the virtual machine. */
void C3DtestApp::vmMessage(TVMmsg msg) {
	if (msg.type == vmMsgString) {
		textWindow->appendText(msg.text);

	}
}

/** Carry out any processing demanded by the virtual machine. */
void C3DtestApp::vmUpdate() {

	if (vm.getStatus() == vmAwaitChoice && !shownChoice) {
		showChoice();
		//getChoice(vm);
	}
	if (vm.getStatus() == vmAwaitString) {
		//getString(vm);
	}
	if (vm.getStatus() == vmExecuting) {
		vm.execute();
	}
	
}

/** Write the user's choices to the choice menu. */
void C3DtestApp::showChoice() {
	choiceMenu->clear();
	std::vector<std::string> optionStrs;
	vm.getOptionStrs(optionStrs);
	for (auto optionStr : optionStrs) {
		choiceMenu->addItem(optionStr);
	}
	choiceMenu->setVisible(true);
	shownChoice = true;
}


void C3DtestApp::HandleUImsg(CGUIbase & Control, CMessage & Message) {
	if (Control.getID() == menuID && Message.Msg == uiMsgLMdown) {
		TVMmsg msg;
		msg.type = vmMsgChoice;
		msg.integer = Message.value;
		shownChoice = false;

	//	textWindow->setTextColour(UIblue);
	//	textWindow->appendText(" Tacked-on text.");

		textWindow->appendText("\n\n");
	//	textWindow->appendText(" ** ");
		textWindow->setTextColour(UIred);
	
		std::vector<std::string> optionStrs;
		vm.getOptionStrs(optionStrs);
		textWindow->appendText(optionStrs[Message.value]);
		textWindow->appendText("\n\n");
	//	textWindow->appendText(" ** ");
		textWindow->setTextColour(UIwhite);

		vm.sendMessage(msg);
	}
}






C3DtestApp::~C3DtestApp() {
	
	//delete terrain;
	//TO DO: since these are created with Engine.getModel, engine should handle deletion.
	//delete chunkShell;
//	delete shaderChunkGrid;
	//delete chunkBB;
//	delete tempFeedbackBuf;
}





