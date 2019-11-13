#include "3DtestApp.h"

#include <glew.h>
#include <algorithm>
#include <string>
#include <vector>
//#include <stdio>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/common.hpp>
#include <glm/gtx/rotate_vector.hpp> 

#include "colour.h"

#include <glm/gtc/matrix_access.hpp>	 //temp

#include "UI\GUIimage.h"
#include "UI\GUIlabel2.h"
#include "UI\GUIcheckButton.h"

#include "plants\fractalTree.h"

#include "shapes.h"


using namespace glm;



C3DtestApp::C3DtestApp() {
	
	tmpSCno = 0;
	tmp = false;
	physCube = NULL;
	shownChoice = false;
	oldPos = vec3(0);
}

void C3DtestApp::onStart() {
	appMode = hexMode;// texGenMode;// terrainMode; //textMode; //hexMode;

	if (appMode == hexMode)
		logWindow->setTextColour(glm::vec4(1));
	
	//GUIroot.add(uiButton, "Save");

	chunkCall = 0;

	dataPath = homeDir + "Data\\";
	lastMousePos = glm::vec2(0, 0);

	


	//test objects, temporary
	//cube = Engine.createCube(vec3(-3, 300, -3), vec3(1.0f)); Engine.modelDrawList.push_back(cube);

	cube.loadMesh(shape::cubeMesh());
	cube.setPos(vec3(-3, 300, -3));
	Engine.modelDrawList.push_back(&cube);

//	Engine.modelDrawList.push_back(Engine.createCube(vec3(3, 300, -3), vec3(1.0f)));
	cube2.loadMesh(shape::cubeMesh());
	cube2.setPos(vec3(3, 300, -3));
	Engine.modelDrawList.push_back(&cube2);


//	Engine.createCylinder(vec3(0, 300, -4), 1, 2, 30);
	cylinder.loadMesh(shape::cylinderMesh(1,2,30));
	cylinder.setPos(vec3(0, 300, -4));
	Engine.modelDrawList.push_back(&cylinder);

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

	renderer.defaultLightPos = vec3(4000, 4000, 4000);
	renderer.defaultLightDir = vec3(0,0,-1);
		
	/*CTerrainPhysObj* terrainPhysObj = new CTerrainPhysObj();
	terrainPhysObj->attachModel(&terrain);
	terrainPhysObj->setCollides(false);
	Engine.physObjManager.addPhysObj(terrainPhysObj);
	*/



	
	
	

	
	
	createRegion();

	
	
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
	//terrain->createShells(5120, 320, 2); //1280 320
	//terrain->createShells(1280, 320, 0);
	terrain.createShells(10000, 320, 2);
	//terrain->createShells(1280, 320, 0);
	//terrain->createShells(8, 2, 2); //(8, 3, 2); //(4,2,1);

	terrain.initChunkShell();
	terrain.initChunkGrid(cubesPerChunkEdge);

	

	terrain.initHeightFinder();
	//terrain.createTerrain(vec2(0.05, 0.05)); //seem to get striations above 999
	terrain.createTerrain(playerStartOffset); //seem to get striations above 999

	initHeightmapGUI();
	

	double t = Engine.Time.milliseconds();
	SCpassed = SCrejected = 0;

	terrain.createAllChunks();


	////////////NEW TERRAIN STUFF
	
	CBaseBuf* terrainBuf2 = &multiBuf; //TO DO: ugh, make a setter
//((CMultiBuf*)terrainBuf)->setRenderer(&Engine.Renderer);

	terrainBuf2->setSize(175000000); //175000000

	terrainBuf2->storeLayout(3, 3, 0, 0);

	tempFeedbackBuf2 = Engine.createBuffer();
	tempFeedbackBuf2->setSize(1000000);



	shellTotalVerts = terrain.shellTotalVerts;
	int LoD1shellSCs = 5;
	terrain2.setSampleSpacePosition(terrain.sampleOffset);
	terrain2.setWorldScale(2560);
	terrain2.setCallbackApp(this);
	terrain2.createLoD1shell(cubeSize, cubesPerChunkEdge, chunksPerSuperChunkEdge, LoD1shellSCs);
	terrain2.addShell(1);
	terrain2.addShell(1);
	terrain2.addShell(2);

	terrain2.fillShells();

	//terrain2.getInnerBounds(1);


	CTerrainPhysObj2* terrainPhysObj2 = new CTerrainPhysObj2();
	terrainPhysObj2->attachModel(&terrain2);
	terrainPhysObj2->setCollides(false);
	Engine.physObjManager.addPhysObj(terrainPhysObj2);


	t = Engine.Time.milliseconds() - t;
	
	skyDome = Engine.createSkyDome();

	oldTime = Engine.Time.seconds();

	initWireSCs();
	supWire = false;

	//initialise player object
	playerObject.model.loadMesh(shape::cubeMesh()); 
	//= Engine.createCube(vec3(0), vec3(playerObject.width * 1, playerObject.height, playerObject.width * 1));
	playerObject.model.scale(vec3(playerObject.width * 1, playerObject.height, playerObject.width * 1));
	playerObject.setPos(vec3(0, 237, 0));
	playerObject.setPos(vec3(-490, 13900, -490));
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

	

	//GUIroot.borderWidth = 10;
	//popFont.loadFromFile(dataPath + "Aeileron18.fnt");
	popFont.loadFromFile(dataPath + "hotMed20.fnt");




	//mainFont.loadFromFile(dataPath + "hotMed26.fnt");
	//mainFontBold.loadFromFile(dataPath + "hotMed26bold.fnt");
	
	//mainFont.loadFromFile(dataPath + "seg22sl.fnt"); ///curreny
	////////mainFont.loadFromFile(dataPath + "robL24.fnt"); //segl24 uneven, better at 26, segsl24 doesn't have this but is a little heavy
	//opl a little scratchy until you get to 24
	//roboto thin s is mangled - vertical error? 
	//roboto light is pretty good though
	//mainFont.loadFromFile(dataPath + "rob22d.fnt");
	mainFont.loadFromFile(dataPath + "merri16L.fnt"); //pretty good light font
	//mainFont.loadFromFile(dataPath + "merri16.fnt"); //regular might be even better
	///mainFont.loadFromFile(dataPath + "work18.fnt"); //too small at 16
	//mainFont.loadFromFile(dataPath + "work18L.fnt"); //not quite... helvetica too old fashioned?

	//mainFont.loadFromFile(dataPath + "rob16.fnt"); 

	//mainFontBold.loadFromFile(dataPath + "rob22m.fnt");
	mainFontBold.loadFromFile(dataPath + "merri16.fnt");
	//mainFontBold.loadFromFile(dataPath + "robtest.fnt");

	//mainFont.loadFromFile(dataPath + "pla26.fon");
	popHeadFont.loadFromFile(dataPath + "hotBold20.fnt");


	renderer.fontManager.createFromFile("mainHeader", dataPath + "merri16.fnt");
	renderer.fontManager.createFromFile("main", dataPath + "merri16L.fnt");

	renderer.fontManager.createFromFile("smallHeader", dataPath + "merri14.fnt");
	renderer.fontManager.createFromFile("small", dataPath + "merri14L.fnt");

	vm.loadProgFile(dataPath + "..\\..\\TC\\Debug\\output.tig");
	//vm.loadProgFile(dataPath + "..\\..\\TC\\output.tig");

	//got here

	worldUI.setGameApp(this);
	worldUI.setVM(&vm);
	
	//worldUI.setTextWindow(textWindow);
	//worldUI.setInventoryWindow(invWindow);
	//worldUI.setPopupTextWindow(popupPanel);
	worldUI.init();
	//didn't get here
	//worldUI.setMainBodyStyle(mainFont, uiDarkGrey);
	//worldUI.setMainHeaderStyle(mainFontBold, uiDarkGrey);

//	worldUI.setMainBodyStyle(renderer.fontManager.getFont("work16L"), uiDarkGrey);
//	worldUI.setMainHeaderStyle(renderer.fontManager.getFont("work16"), uiDarkGrey);



//	worldUI.setInvBodyStyle(popFont, white);
//	worldUI.setPopBodyStyle(popFont, white);
//	worldUI.setPopHeaderStyle(popHeadFont, white);
	//worldUI.setHottextColour(hot);
	//worldUI.setHottextSelectColour(hotSelect);

	worldUI.start();

	//texCompositor.init(this);
	//texCompositor.initTex();
	//texCompositor.compose();
	//texCompositor.colourise();

	//got here 2

	texGenUI.init(this);


	texGenUI.compose();
	//still did not get here



	if (appMode != texGenMode)
		texGenUI.hide(true);
	if (appMode != textMode)
		worldUI.hide(true);

	tmpModel2.loadMesh(shape::cubeMesh());
	tmpModel2.setPos(vec3(0));

	tmpModel2.scale(vec3(3000, 3000, 3000));
	tmpModel2.rotate(45, vec3(1, 0, 0));

	initHexWorld();
	
	return;
}






/** Called every frame, provides a place for the user to check input where constant feedback is required. */
void C3DtestApp::keyCheck() {
	if (appMode == hexMode) {
		hexWorld.keyCheck();
	}


	if (appMode == terrainMode) {

		CCamera* currentCamera = Engine.getCurrentCamera();
		float moveInc = float(dT * 1000.0); // 0.05125f;

		if (keyNow('L')) {
			renderer.defaultLightPos = rotate(renderer.defaultLightPos, 0.01f, vec3(0, 1, 0));
			//building.setPos(renderer.defaultLightPos);
		}

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
				currentCamera->yaw(float(-yawAng * dT));
			}

			float rot = dT * 200.0;
			if (keyNow('P')) {
				cube.rotate(rot, glm::vec3(1, 0, 0));
			}
			if (keyNow('Y')) {
				cube.rotate(rot, glm::vec3(0, 1, 0));
			}
	


		}
		else {
			vec3 moveDir(0);
			//if (length(playerPhys->currentContactNormal) <= 0) {
			//TERRAIN2 this prevents WASD ever getting checked
			if (false) {
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
					groundNormal = vec3(0, 1, 0);
					//TERRAIN2 would normally be set by physics/collision
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
		if (keyNow('7')) {
			advance(up);
		}

		selectChk = glm::mod(vec3(selectChk), vec3(15, 5, 15));


		if (mouseKey == MK_LBUTTON /*&& !worldUI.popupPanel->getVisible()*/) //TO DO: make less kludgy 
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
				setMousePos((int)oldMousePos.x, (int)oldMousePos.y);
				showMouse(true);

			}

		}

	}


	if (appMode == texGenMode) {
		if (keyNow('R')) {
			texGenUI.compose();

		}


	}
	

}



/** Triggered *when* a key is pressed, not while it is held down. This is not 'whileKeyDown'. */
void C3DtestApp::onKeyDown( int key, long mod) {
	if (appMode == hexMode) {
		hexWorld.onKeyDown(key, mod);
		return;
	}


	if (key == 'R' && mod == GLFW_MOD_CONTROL) {
		if (appMode == textMode) {
			vm.reset();
			vm.reloadProgFile();
			worldUI.reset();
			return;
		}

		if (appMode == terrainMode) {
			terrain.clear();
			terrain.chunkShader->recompile();
			terrain.chunkCheckShader->recompile();
			terrain2texShader->recompile();
			terrain.createAllChunks();
			updateHeightmapImage();
			//terrain.grassShader->recompile();
			//EatKeys();
		}

	}


	

	if (key == '1') {
		fpsOn = !fpsOn;
		if (fpsOn) {
			
			renderer.setCurrentCamera(&playerObject.povCam);
			playerPhys->asleep = false;
		}
		else
			renderer.setCurrentCamera(renderer.defaultCamera);
		//EatKeys();

	}

	if (key == 'X') {
		/*	terrain->clear();
		chunkShader->recompile();
		terrain2texShader->recompile();
		terrain->createAllChunks();
		updateHeightmapImage();
		*/
		for (unsigned int s = 0; s < terrain.shells[3].superChunks.size(); s++) {
			CSuperChunk* sc = terrain.shells[3].superChunks[s];
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

	if (appMode == hexMode) {
		Engine.Renderer.setBackColour((rgba&)uialmostBlack);
		Engine.Renderer.clearFrame();
		hexWorld.draw();
		return;
	}
	
	if (appMode != terrainMode) {
		Engine.Renderer.setBackColour((rgba&)uialmostBlack);
		Engine.Renderer.setBackColour((rgba&)white);
		Engine.Renderer.clearFrame();
		return;
	}

	///////////////////////////tmpModel2.draw();
	building.draw();

	mat4 fpsCam = playerObject.povCam.clipMatrix;// *terrain->chunkOrigin;
	terrain.updateVisibleSClist(fpsCam);

	double t = Engine.Time.milliseconds();

	mat4 mvp = Engine.getCurrentCamera()->clipMatrix * terrain.chunkOrigin;

	//terrain.drawVisibleChunks();/////////////////////////////
	drawVisibleChunks();

	//draw grass
	//TO DO: temporarily commented out while I remove Soil
	Engine.Renderer.setShader(terrain.grassShader);
//	Engine.Renderer.attachTexture(0, *terrain.grassTex);
	terrain.grassShader->setTextureUnit(0, terrain.hGrassTexure);
	terrain.grassShader->setShaderValue(terrain.hGrassTime, (float)Time);
	terrain.grassShader->setShaderValue(terrain.hGrassMVP, mvp);
	//	terrain.drawGrass(mvp, terrain.visibleSClist);


		//draw trees
	Engine.Renderer.setShader(Engine.Renderer.phongShaderInstanced);
	Engine.Renderer.phongShaderInstanced->setShaderValue(Engine.Renderer.hPhongInstNormalModelToCameraMatrix, tmp);
	Engine.Renderer.phongShaderInstanced->setShaderValue(Engine.Renderer.hPhongInstMVP, mvp);

	//glEnable(GL_PRIMITIVE_RESTART);

	//	terrain.drawTrees(mvp, terrain.visibleSClist);

	//glDisable(GL_PRIMITIVE_RESTART);

	t = Engine.Time.milliseconds() - t;


	//wireframe drawing:
	//draw bounding boxes
	if (supWire) {
		int component = 1;
		CSuperChunk* sc;
		vec4 colour = vec4(0, 0, 0, 1);
		vec3 cornerAdjust, opCornerAdjust;
		Engine.Renderer.setShader(terrain.wireBoxShader);
		for (int l = terrain.shells.size() - 1; l > -1; l--) {
			vBuf::T3DnormVert box[2000]; //should be enough
			int index = 0;
			for (unsigned int s = 0; s < terrain.shells[l].superChunks.size(); s++) {
				sc = terrain.shells[l].superChunks[s];
				if (sc->nonEmpty)
				{
					box[index].v = sc->nwWorldPos + terrain.shells[l].nwLayerPos;
					cornerAdjust = vec3(sc->faceBoundary[west], sc->faceBoundary[down], sc->faceBoundary[north]);
					box[index].v += cornerAdjust * sc->chunkSize;

					opCornerAdjust = vec3(sc->faceBoundary[east] + 1, sc->faceBoundary[up] + 1,
						sc->faceBoundary[south] + 1);
					opCornerAdjust -= cornerAdjust;
					box[index].normal = opCornerAdjust * sc->chunkSize;
					index++;
				}
			}
			wireSCs->storeVertexes(box, sizeof(vBuf::T3DnormVert), index);
			wireSCs->storeLayout(3, 3, 0, 0);
			mvp = Engine.getCurrentCamera()->clipMatrix * wireSCs->worldMatrix;
			terrain.wireBoxShader->setShaderValue(terrain.hBoxMVP, mvp);

			colour[component++] = 0.8f;
			if (component > 2)
				component = 0;
			terrain.wireBoxShader->setShaderValue(terrain.hBoxColour, colour);
			wireSCs->drawNew();
		}
	}


	terrain2TestDraw();

	if (!fpsOn) {
		playerObject.model.draw();
	}

	///watch::watch1 << "text";
}

void C3DtestApp::terrain2TestDraw() {
	//return;
	//draw shell wireframes
	mat4 chunkM;
	float boxScale = 0.99f;

	for (int shell = 0; shell <2; shell++) {
	//int shell = 0;
		vec3 shellWorldspacePos = terrain2.shells[shell].worldSpacePos;
		renderer.setShader(wire2Shader);
		float shellSize = terrain2.getShellSize(shell);
		glm::mat4 shape = glm::scale(glm::mat4(1), glm::vec3(shellSize));
		mat4 scM = translate(mat4(1), shellWorldspacePos);
		glm::mat4 wireCubeMVP = Engine.getCurrentCamera()->clipMatrix * scM * shape;
		wire2Shader->setShaderValue(hWireMVP, wireCubeMVP);
		wire2Shader->setShaderValue(hWireColour, vec4(1, 0, 0, 1));
		renderer.drawBuf(wireCube, drawLinesStrip);

		//now draw boxes for each SC
		float SCsize = terrain2.shells[shell].SCsize * boxScale;
		float scActualSize = terrain2.shells[shell].SCsize;
		glm::mat4 SCshape = glm::scale(glm::mat4(1), glm::vec3(SCsize));

		float chunkSize = terrain2.shells[shell].chunkSize * boxScale;
		float actualChunkSize = terrain2.shells[shell].chunkSize;
		glm::mat4 chunkShape = glm::scale(glm::mat4(1), glm::vec3(chunkSize));
		
		//create world displacement matrix for each SC:
		//find the centre point of the SC using x,y,z index * SCsize
		//make it relative to the shell's origin
		//that's our matrix
		//float scSize = terrain2.shells[shell].SCsize;

		for (COuterSCIterator SCiter = terrain2.shells[shell].getOuterSCiterator(); !SCiter.finished(); SCiter++) {
			if (SCiter->isEmpty)
				continue;

			i32vec3 index = SCiter.getIndex();
			i32vec3 origIndex = SCiter->origIndex;
			vec3 SCorigin = vec3(index) * scActualSize;
			SCorigin += scActualSize * 0.5; //move orgin to centre of SC
			SCorigin -= shellSize * 0.5; // and make relative to centre of shell
			SCorigin += shellWorldspacePos; //and then relative to shell's worldspace positionHint
			scM = translate(mat4(1), SCorigin);
			wireCubeMVP = Engine.getCurrentCamera()->clipMatrix * scM * SCshape;
			wire2Shader->setShaderValue(hWireMVP, wireCubeMVP);
			wire2Shader->setShaderValue(hWireColour, terrain2.shells[shell].scArray.element(index.x, index.y, index.z).colour);
			if (shell == 0 || shell == 1)
				 renderer.drawBuf(wireCube, drawLinesStrip);

			//draw chunks
			//if (shell != 3 && shell !=2 )
			//	 continue;
			for (auto chunk : SCiter->scChunks) {
				i32vec3 chunkIndex = terrain2.chunks[chunk].index;
				vec3 chunkOrigin = vec3(chunkIndex) * actualChunkSize;
				chunkOrigin += actualChunkSize * 0.5; //move orgin to centre of chunk
				chunkOrigin -= scActualSize * 0.5; // and make relative to centre of SC ???? may need actual SC size
				chunkOrigin += SCorigin; //and then relative to shell's worldspace positionHint

				chunkM = translate(mat4(1), chunkOrigin);
				wireCubeMVP = Engine.getCurrentCamera()->clipMatrix * chunkM * chunkShape;
				wire2Shader->setShaderValue(hWireMVP, wireCubeMVP);
				wire2Shader->setShaderValue(hWireColour, terrain2.shells[shell].scArray.element(index.x, index.y, index.z).colour);
				//renderer.drawBuf(wireCube, drawLinesStrip);
			}

		}
		


		/*

		wireCubeMVP = Engine.getCurrentCamera()->clipMatrix * SCshape;
		wire2Shader->setShaderValue(hWireMVP, wireCubeMVP);
		wire2Shader->setShaderValue(hWireColour, vec4(1, 0, 0, 1));
		renderer.drawBuf(wireCube, drawLinesStrip);
		*/


		//draw LoD1 chunk extent as a semi-solid boxs
		//first, find our 6 planes
		float n, e, s, w, u, d;

		CShell* pLoD1Shell = &terrain2.shells[shell];
		n = -pLoD1Shell->chunkExtent[0] * pLoD1Shell->chunkSize;
		e = pLoD1Shell->chunkExtent[1] * pLoD1Shell->chunkSize;
		s = pLoD1Shell->chunkExtent[2] * pLoD1Shell->chunkSize;
		w = -pLoD1Shell->chunkExtent[3] * pLoD1Shell->chunkSize;
		u = pLoD1Shell->chunkExtent[4] * pLoD1Shell->chunkSize;
		d = -pLoD1Shell->chunkExtent[5] * pLoD1Shell->chunkSize;

		//create the corner verts
		vec3 A(w, u, s); //A
		vec3 B(e, u, s); //B
		vec3 C(e, d, s); //C
		vec3 D(w, d, s); //D
		vec3 E(w, u, n); //E
		vec3 F(e, u, n); //F
		vec3 G(e, d, n); //G
		vec3 H(w, d, n); //H

		vector<vec3> verts = { A, B, C, D, //front face
			B, F, G, C, //right face
			F, E, H, G, //back face
			E, A, D, H, //left face
			E, F, B, A, //top face
			D, C, G, H }; //bottom face

		vector<vec3> normals(24);
		normals[0] = normals[1] = normals[2] = normals[3] = glm::vec3(0, 0, 1);
		normals[4] = normals[5] = normals[6] = normals[7] = glm::vec3(1, 0, 0);
		normals[8] = normals[9] = normals[10] = normals[11] = glm::vec3(0, 0, -1);
		normals[12] = normals[13] = normals[14] = normals[15] = glm::vec3(-1, 0, 0);
		normals[16] = normals[17] = normals[18] = normals[19] = glm::vec3(0, 1, 0);
		normals[20] = normals[21] = normals[22] = normals[23] = glm::vec3(0, -1, 0);

		//create index
		vector<unsigned int> index = { 1, 0, 3, 1, 3, 2,
				5, 4, 7, 5, 7, 6,
				9, 8, 11, 9, 11, 10,
				13, 12, 15, 13, 15, 14,
				16, 19, 18, 18, 17, 16,
				21, 20, 23, 21, 23, 22 };

		CBuf box;

		unsigned int nVerts = verts.size();
		//	box.storeVertexes(verts.data(),sizeof(vec3) * nVerts, nVerts);
		box.storeVertexes(verts, normals);
		box.storeIndex(index.data(), index.size());
		box.storeLayout(3, 3, 0, 0);

		mat3 tmp;
		mat4 mvp = Engine.getCurrentCamera()->clipMatrix;
		scM = translate(mat4(1), shellWorldspacePos);
		mvp = mvp * scM;
		Engine.Renderer.setShader(Engine.Renderer.phongShader);
		Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hNormalModelToCameraMatrix, tmp); //why am I doing this?
		Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hMVP, mvp);
		Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hMatDiffuse, vec4(1, 0, 0, 0.25));
		//Engine.Renderer.drawBuf(box, drawTris);
	}
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
	vec3 movement = dir *  float(1.0f);  //was 10
	//terrain->translate(movement);
	//vec3 pos = terrain->getPos();
	//terrain->chunkOrigin[3] += vec4(movement, 0);

	terrain.scrollTriggerPoint += vec3(movement);
	vec3 pos = terrain.scrollTriggerPoint;


	float chunkDist = cubesPerChunkEdge * cubeSize; //span of a chunk in world space
	bvec3 outsideChunkBoundary = glm::greaterThan(glm::abs(pos), vec3(chunkDist, chunkDist, chunkDist));

	//If terrain has moved by the length of a chunk
	if (glm::any(outsideChunkBoundary)) {
		vec3 posMod;
		posMod = glm::mod(pos, vec3(chunkDist, chunkDist, chunkDist)); //glm::mod seems to turn negative remainders positive
		posMod.x = (float)fmod(pos.x, chunkDist);
		posMod.y = (float)fmod(pos.y, chunkDist);
		posMod.z = (float)fmod(pos.z, chunkDist);
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

	///////////////////////////////////////////////////////////////////////////////////
	vec3 playerMovement = dirToVec(direction) * 1.0f;  

	terrain2.playerWalk(playerMovement);

	//TERRAIN2
}

/** Called every frame. Mainly use this to scroll terrain if we're moving in first-person mode*/
void C3DtestApp::Update() {
	vmUpdate();
	
	worldUI.update((float)dT);

	if (skyDome)
		skyDome->update(dT);

	//terrain.update(); //commented out to speed up tests on terrain2

	terrain2.update(dT);

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
		///true if player has moved >chunkDist away from origin

		/////////////////////////////////////////////////////////////
		vec3 dPos = pos - oldPos;
		oldPos = pos;
		terrain2.playerWalk(dPos);
		///tell the terrain about the move so that it can scroll etc
		//return;

		//TERRAIN2
		//jumping back problem happens somewhere below, but without it player object just gets further 
		//and further from origin. Obviously this needs to be fixed.
		//player object should move when terrain moves, and by same amount
		//use terrain 2 to tell us when it has moved, then move player accordingly, that's simplest

		//has viewpoint moved beyond the length of one chunk?
		if (outsideChunkBoundary.x || outsideChunkBoundary.y || outsideChunkBoundary.z) {
			vec3 posMod;
			posMod.x = fmod(pos.x, chunkDist);
			posMod.y = fmod(pos.y, chunkDist);  //was pos.y
			posMod.z = fmod(pos.z, chunkDist);
			//posMod = extent of player position beyond centrol chunk
			
			//WAS
			//fpsCam.setPos(posMod); //secretly reposition viewpoint prior to scrolling terrain
			//playerObject.setPos(posMod);
			//playerPhys->position = posMod;
			///player now repositioned within central chunk
			///problem triggered by the above line
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
				//terrain.advance(direction);
				//return;
			}

			if (outsideChunkBoundary.y) {
				if (pos.y > 0)
					direction = up;
				else
					direction = down;
				terrain.chunkOriginInt += dirToVec(flipDir(direction));
				terrain.chunkOrigin[3] = vec4(chunkDist * vec3(terrain.chunkOriginInt), 1);
			//	terrain.advance(direction);
				//return;
			}


			if (outsideChunkBoundary.z) {
				if (pos.z > 0)
					direction = south;
				else
					direction = north;
				terrain.chunkOriginInt += dirToVec(flipDir(direction));
				terrain.chunkOrigin[3] = vec4(chunkDist * vec3(terrain.chunkOriginInt) , 1);
			//	terrain.advance(direction);
			}

			onTerrainAdvance(direction);

		}
	}
}



/** Create a model for drawing superChunk positions in wireframe. */
void C3DtestApp::initWireSCs() {
	wireSCs = Engine.createModel();
	
	wireSCs->drawMode = GL_POINTS;
	wireSCs->getMaterial()->setShader(terrain.wireBoxShader);

	wireSCs->setPos(vec3(0));

	//create a cube to do the same thing for terrain2
	std::vector<glm::vec3> verts;
	shape::cubePoints(&verts, NULL);
	unsigned int index[] = { 0,1,2,3,0,4,5,6,7,4,7,3,2,6,5,1};
	wireCube.storeVertexes(verts.data(), 12 * verts.size(), verts.size());
	wireCube.storeIndex(index, 16);
	wireCube.storeLayout(3, 0, 0, 0);
	
	wire2Shader = renderer.createShader("wire2");
	hWireMVP = wire2Shader->getUniformHandle("mvpMatrix");
	hWireColour = wire2Shader->getUniformHandle("colour");
	//wireCubeVerts = verts;
}

/**	Called when terrain advances - ie, moves. */
void C3DtestApp::onTerrainAdvance(Tdirection direction) {
	if (physCube) {
	//	physCube->pModel->translate(-dirToVec(direction) * (float)cubesPerChunkEdge * cubeSize);
		//physCube->positionHint += -dirToVec(direction) * (float)cubesPerChunkEdge * cubeSize;
	}
	//playerObject.translate(-dirToVec(direction) * (float)cubesPerChunkEdge * cubeSize);
//	building.translate(-dirToVec(direction) * (float)cubesPerChunkEdge * cubeSize);
	//liveLog << "\nonTerrainAdvance called!";
}

/** Create the GUI and textures for displaying 2D terrain heightmaps. */
void C3DtestApp::initHeightmapGUI() {
	heightmapImage = new CGUIimage(10, 10, 500, 500); // 500, 500);
	GUIroot.Add(heightmapImage);
	heightmapTex = Engine.Renderer.textureManager.createEmptyTexture(128, 221);// 500, 500);
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

	//heightmapImage->setTexture((CBaseTexture&)mainFont.texture);
	heightmapImage->setTexture((CBaseTexture&)terrain.tmpTerrainMap);

	return;


	heightmapImage->setTexture(*heightmapTex);
	Engine.Renderer.setShader(terrain2texShader);
	vec3 sampleCorner = terrain.shells[0].nwSampleCorner;
	//terrain2texShader->setNwSampleCorner(vec2(sampleCorner.x,sampleCorner.z));
	terrain2texShader->setShaderValue(hTer2TexNwSampleCorner, vec2(sampleCorner.x, sampleCorner.z));

	//calculate scaling
	float scale = (terrain.worldSize.x / terrain.worldUnitsPerSampleUnit) / heightmapTex->width;

	//terrain2texShader->setPixelScale(scale);
	terrain2texShader->setShaderValue(hTer2TexPixelScale, scale);

	Engine.Renderer.renderToTextureQuad(*heightmapTex);

	
}


/** Handle messages from the virtual machine. */
void C3DtestApp::vmMessage(TvmAppMsg msg) {
	if (msg.type == appFlush) {
		worldUI.flushMessageQueue();
		return;
	}

	worldUI.queueMsg(msg);
}

/** Carry out any processing demanded by the virtual machine. */
void C3DtestApp::vmUpdate() {

	/*if (vm.getStatus() == vmAwaitChoice && !shownChoice) {
		showChoice();
		//getChoice(vm);
	} */
	if (vm.getStatus() == vmAwaitString) {
		//getString(vm);
	}
	if (vm.getStatus() == vmExecuting) {
		vm.execute();
	}
	
}




void C3DtestApp::HandleUImsg(CGUIbase & control, CMessage & Message) {


	if (control.parent->getUniqueID() == worldUI.invPanelID && Message.Msg == uiMsgHotTextClick) {
		glm::i32vec2 mousePos = glm::i32vec2(Message.x, Message.y);
		worldUI.inventoryClick(Message.value, mousePos);
		return;
	}

	//popup menu text click
	if (control.parent->id == popMenuId && Message.Msg == uiMsgHotTextClick) {
		glm::i32vec2 mousePos = control.localToScreenCoords(Message.x, Message.y);
		worldUI.menuClick(Message.value, mousePos, (CGUIrichTextPanel *)control.parent);
		return;
	}

	if (control.parent->id == popMenuId && Message.Msg == uiClickOutside) {
		glm::i32vec2 mousePos = control.localToScreenCoords(Message.x, Message.y);
		worldUI.deletePopupMenu((CGUIrichTextPanel*)control.parent);
		return;
	}

	if (control.parent->id == popMenuId && Message.Msg == uiMsgMouseMove) {
		worldUI.queueMsg(TvmAppMsg{ appHotTxtChange,"","",Message.value });;
		return;
	}

	
	if (control.parent->id == popMenuId && Message.Msg == uiMouseWheel) {
		worldUI.mouseWheelHotText(Message.value, Message.value2);
		return;
	}



}

/** Returns true if terrain *does not* intersect the given cube - ie, SC is empty. */
bool C3DtestApp::scIntersectionCheckCallback(glm::vec3 & nwSamplePos, float SCsampleStep) {
	Engine.Renderer.setShader(terrain.chunkCheckShader);
	//find nwcorner in sample space
	

	//float LoDscale = (SC.sampleStep) / (cubesPerChunkEdge);
	float chunkShellVertCount = (float)terrain2.chunkCubes;
	//This is simply the number of divisions chunkShell has, because normally it is used to check MC cube
	//points on the surface of a chunk-sized volume. 
	//Here we are abusing it to check points on the surface of a SC-sized volume.
	//TO DO: create a separate shell for SC checks, to avoid this ambiguity

	terrain.chunkCheckShader->setShaderValue(terrain.hNWsamplePos, nwSamplePos);
	terrain.chunkCheckShader->setShaderValue(terrain.hLoDscale, SCsampleStep/ chunkShellVertCount);

	Engine.Renderer.attachTexture(0, terrain.tmpTerrainMap.handle);
	terrain.chunkCheckShader->setShaderValue(terrain.hTerrainTexture, 0);

	Engine.Renderer.initQuery();
	terrain.chunkShell->drawNew();
	unsigned int primitives = Engine.Renderer.query();

	//TO DO: chunkshell is coarse, create a SCshell with more points
	if ((primitives == 0) || (primitives == shellTotalVerts)) {
		return true; //outside surface
	}

	return false;
}

//TO DO: these two methods are identical aside from returning true/false. Consolidate!

/** Returns true if terrain *intersects* the given chunk volume .*/
bool C3DtestApp::chunkCheckCallback(glm::vec3 & chunkSamplePos, float chunkSampleSize) {
	Engine.Renderer.setShader(terrain.chunkCheckShader);
	
	float chunkShellVertCount = (float)terrain2.chunkCubes; ///<Gives number of divisions in the shell
	
	terrain.chunkCheckShader->setShaderValue(terrain.hNWsamplePos, chunkSamplePos);
	terrain.chunkCheckShader->setShaderValue(terrain.hLoDscale, chunkSampleSize / chunkShellVertCount);

	Engine.Renderer.attachTexture(0, terrain.tmpTerrainMap.handle);
	terrain.chunkCheckShader->setShaderValue(terrain.hTerrainTexture, 0);

	//Draw check grid 
	//unsigned int primitives = Engine.drawModelCount(*chunkShell);

	Engine.Renderer.initQuery();
	terrain.chunkShell->drawNew();
	unsigned int primitives = Engine.Renderer.query();


	if (primitives == 0 || primitives == shellTotalVerts)
		return false;
	
	return true;
}

/** copy the data from the multibuf to system memory */
unsigned int C3DtestApp::getChunkTrisCallback(int chunkId, TChunkVert* buf) {
	multiBuf.copyBlock(chunkId, (char*)buf);
	return multiBuf.getBlockSize(chunkId);
}


/*  Create a mesh for this chunk, and register it with the renderer.  */
void C3DtestApp::createChunkMesh(Chunk2& chunk) {
	Engine.Renderer.setShader(terrain.chunkShader);
	terrain.chunkShader->setShaderValue(terrain.hChunkCubeSize, chunk.cubeSize);

	float LoDscale = chunk.LoD;
	terrain.chunkShader->setShaderValue(terrain.hChunkLoDscale, LoDscale);
	terrain.chunkShader->setShaderValue(terrain.hChunkSamplePos, chunk.sampleCorner);

	//sysLog << "\nLod " << chunk.LoD << " cubesize " << chunk.cubeSize;

	float samplesPerCube = cubeSize / terrain.worldUnitsPerSampleUnit;
	terrain.chunkShader->setShaderValue(terrain.hSamplesPerCube, samplesPerCube);

	terrain.chunkShader->setShaderValue(terrain.hChunkTriTable, *terrain.triTableTex); 
	terrain.chunkShader->setShaderValue(terrain.hChunkTerrainPos, chunk.terrainPos);

	Engine.Renderer.attachTexture(0, terrain.tmpTerrainMap.handle);
	terrain.chunkShader->setShaderValue(terrain.hChunkTerrainTexture, 0);

	int chunkAttribs = 2;
	int vertsPerPrimitive = 3 * chunkAttribs;
	int maxMCverts = 16; //The maximum vertices needed for a surface inside one MC cube.
	int nVertsOut = cubesPerChunkEdge * cubesPerChunkEdge * cubesPerChunkEdge * maxMCverts;

	CBaseBuf* terrainBuf = &multiBuf;
	CBuf* srcBuf = &((CRenderModel*)terrain.shaderChunkGrid)->buf;
	unsigned int primitives = Engine.Renderer.getGeometryFeedback(*srcBuf, drawLinesAdjacency, (CBuf&)* tempFeedbackBuf2, drawTris);


	if (primitives) {
		int outSize = primitives * 3 * sizeof(vBuf::T3DnormVert);
		totalbufsize += outSize;
		totalchunks++;

		terrainBuf->copyBuf(*tempFeedbackBuf2, outSize);


		chunk.bufId = terrainBuf->getLastId();

		TDrawDetails* details = &chunk.drawDetails;
		terrainBuf->getElementData(chunk.bufId, details->vertStart, details->vertCount, details->childBufNo);
		details->colour = chunk.colour;

	//	if (chunk.LoD == 1) {
	//		findTreePoints(chunk);
	//		//	findGrassPoints(chunk);
	//	}
	}
	else
		chunk.bufId = NULL;

	chunk.status = chSkinned;



	//totalTris += (primitives * 3);
}


void C3DtestApp::deleteChunkMesh(Chunk2& chunk) {
	CBaseBuf* terrainBuf = &multiBuf;

	//if (chunk.bufId != 0) //TO DO: temp bug tracking! Should never happen!
		terrainBuf->deleteBlock(chunk.bufId);
	chunk.bufId = 0;
}

void C3DtestApp::drawVisibleChunks() {
	mat4 mvp = Engine.Renderer.currentCamera->clipMatrix * terrain2.chunkOrigin;

	//draw chunks
	Engine.Renderer.setShader(Engine.Renderer.phongShader);
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hMVP, mvp);
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hModel, mat4(1));
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hView, Engine.Renderer.currentCamera->camMatrix);
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hLightPosition, Engine.Renderer.defaultLightPos);

	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hLightSpecular, glm::vec4(1));
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hLightDiffuse, glm::vec4(0.8f, 0.8f, 0.8f, 1));
	Engine.Renderer.phongShader->setShaderValue(Engine.Renderer.hLightAmbient, glm::vec4(0.2f, 0.2f, 0.2f, 1));

	Engine.Renderer.attachTexture(0, Engine.Renderer.texture1x1->handle);
	



//	CSuperChunk* sc;
//	int childBuf = -1;
//	int scListSize = visibleSClist.size();
//	for (int scNo = 0; scNo < scListSize; scNo++) {
	//	sc = visibleSClist[scNo];
	//	int clSize = sc->chunkList.size();
	//	for (int chunkNo = 0; chunkNo < clSize; chunkNo++) {
		for (int chunkNo = 0; chunkNo < terrain2.chunks.size(); chunkNo++) {
			Chunk2* chunk = &terrain2.chunks[chunkNo];
			if (chunk->status != chSkinned)
				continue;
			terrain.chunkDrawShader->setShaderValue(Engine.Renderer.hMatDiffuse, chunk->drawDetails.colour);
			terrain.chunkDrawShader->setShaderValue(Engine.Renderer.hMatAmbient, chunk->drawDetails.colour);
			terrain.chunkDrawShader->setShaderValue(Engine.Renderer.hMatSpecular, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			terrain.chunkDrawShader->setShaderValue(Engine.Renderer.hMatShininess, 32.0f);
			Engine.Renderer.drawMultiBufChildVerts(drawTris, multiBuf, chunk->drawDetails.childBufNo, chunk->drawDetails.vertStart, chunk->drawDetails.vertCount);

		}


	//}
}

/** Create a playable terrain region, with a start point, end point and path. */
void C3DtestApp::createRegion() {
	testCompositor.initTex();
	testCompositor.restore(dataPath + "terrainTest.gen");
	testCompositor.compose();
	terrain.tmpTerrainMap = testCompositor.getComposedTexture();
	playerStartOffset = static_cast<CTerrainTex*>(testCompositor.currentTexGen)->getStartPoint();
	vec2 endPoint = static_cast<CTerrainTex*>(testCompositor.currentTexGen)->getEndPoint();

	playerStartOffset *= 0.5;
	endPoint *= terrain.worldUnitsPerSampleUnit *0.5;

	//create placeholder building
	ComposeTest buildingTexCompositor;
	buildingTexCompositor.initTex();
	buildingTexCompositor.restore(dataPath + "buildingTex.gen");
	buildingTexCompositor.compose();
	tmpBuildingTexture = buildingTexCompositor.getComposedTexture();


	building.loadMesh(shape::fustrumMesh(0.5f));
	building.setPos(vec3(endPoint.x,90, endPoint.y));
	building.scale(vec3(180));
	building.setTexture(&tmpBuildingTexture);



}

/** Called when the terrain scrolls. Useful for moving the rest of the world
	to keep it in line.*/
void C3DtestApp::onTerrainScroll(glm::vec3& movement) {
	liveLog << "\nThe earth moved " << movement;
	building.translate(movement);
	playerObject.translate(movement);
	oldPos += movement;
	playerPhys->position += movement;
}

void C3DtestApp::onResize(int width, int height) {
	hexWorld.setAspectRatio(glm::vec2(width, height));
}

/** Get the hexWorld ready for use. */
void C3DtestApp::initHexWorld() {
	hexWorld.setCallbackApp(this);
	importer.loadFile(dataPath + "models\\test.obj");
	hexWorld.addMesh("test",importer.getMeshes());
	//... more models

	hexWorld.start();
}



/** Trap mousewheel events for our own use. */
void C3DtestApp::OnMouseWheelMsg(float xoffset, float yoffset) {
	int x, y;
	win.getMousePos(x, y);
	int delta = int(yoffset);
	int keyState = 0; //can get key state if ever needed

	CBaseApp::OnMouseWheelMsg(xoffset, yoffset);

	if (appMode == hexMode)
		hexWorld.onMouseWheel(yoffset);
}




C3DtestApp::~C3DtestApp() {
	
	//delete terrain;
	//TO DO: since these are created with Engine.getModel, engine should handle deletion.
	//delete chunkShell;
//	delete shaderChunkGrid;
	//delete chunkBB;
//	delete tempFeedbackBuf;
}





