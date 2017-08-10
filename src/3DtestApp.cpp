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

using namespace watch;

using namespace glm;



C3DtestApp::C3DtestApp() {

	tmpSCno = 0;
	tmp = false;
	physCube = NULL;
	dummy2 = NULL;
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
	Engine.getCurrentCamera()->setPos(vec3(0, 303, 6));
	Engine.getCurrentCamera()->lookAt(vec3(0, -1, -3));


	Engine.getCurrentCamera()->setPos(vec3(-108.678, 1810.77, - 3356.29));
	Engine.getCurrentCamera()->lookAt(vec3(0.0281552, - 0.573389, 0.818797));


	//Position FPS camera
	fpsCam.setPos(vec3(0, 180, 0));
	fpsCam.lookAt(vec3(0, -1, -3));
	fpsOn = false;
	selectChk = i32vec3(0, 0, 0);
	mouseLook = false;

	terrain = Engine.createTerrain();
	CBaseBuf* terrainBuf = &terrain->multiBuf;
	terrainBuf->setSize(175000000);

	terrainBuf->storeLayout(3, 3, 0, 0);

	tempFeedbackBuf = Engine.createBuffer();
	tempFeedbackBuf->setSize(1000000);


	terrain->EXTsuperChunkIsEmpty.Set(this, &C3DtestApp::superChunkIsEmpty);
	terrain->EXTchunkExists.Set(this, &C3DtestApp::chunkExists);
	terrain->EXTfreeChunkModel.Set(&Engine, &CEngine::freeModel);
	terrain->EXTcreateChunkMesh.Set(this, &C3DtestApp::createChunkMesh);



	//load chunkCheck shader
	chunkCheckShader = new ChunkCheckShader();
	chunkCheckShader->pRenderer = &Engine.Renderer;
	chunkCheckShader->create(dataPath + "chunkCheck");
	chunkCheckShader->getShaderHandles();

	initChunkShell();
	initChunkGrid(cubesPerChunkEdge);

	terrain->setSizes(chunksPerSuperChunkEdge, cubesPerChunkEdge, cubeSize);

	initHeightFinder();

	//terrain->createLayers(5120, 320, 2); //1280 320
	//terrain->createLayers(1280, 320, 0);
	terrain->createLayers(10000, 320, 2);
	//terrain->createLayers(1280, 320, 0);
	//terrain->createLayers(8, 2, 2); //(8, 3, 2); //(4,2,1);

	createTerrain(vec2(6, 6)); //seem to get striations above 999

	initHeightmapGUI();
	

	double t = Engine.Time.milliseconds();
	SCpassed = SCrejected = 0;

	terrain->createAllChunks();

	t = Engine.Time.milliseconds() - t;
	cerr << "\n time " << t << " SCs rejected " << SCrejected << " SCs passed " << SCpassed;


	chunkShader = new ChunkShader();
	chunkShader->feedbackVaryings[0] = "gl_Position";
	chunkShader->feedbackVaryings[1] = "normal";
	Engine.shaderList.push_back(chunkShader);
	chunkShader->pRenderer = &Engine.Renderer;
	chunkShader->load(vertex, dataPath + "chunk.vert");
	chunkShader->load(geometry, dataPath + "chunk.geom");
	chunkShader->attach();
	chunkShader->setFeedbackData(2);
	chunkShader->link();

	//Upload data texture for chunk shader
	triTableTex = Engine.createDataTexture(intTex, 16, 256, &triTable);

	Engine.Renderer.setShader(chunkShader);
	chunkShader->getShaderHandles();
	chunkShader->setChunkTriTable(*triTableTex);

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

	
	initGrassFinding();
	
	dummy2 = Engine.createBuffer();
	vec3 v(1);
	unsigned short index = 0;
	dummy2->storeVertexes(&v, sizeof(vec3), 1);
	dummy2->storeIndex(&index, sizeof(index), 1);
	dummy2->storeLayout(3, 0, 0, 0);

	return;
}



/*  Create a mesh for this chunk, and register it with the renderer.  */
void C3DtestApp::createChunkMesh(Chunk& chunk) {
	chunkCall++;

	Engine.Renderer.setShader(chunkShader);
	chunkShader->setChunkCubeSize(chunk.cubeSize);


	float LoDscale = float(1 << chunk.LoD - 1);
	chunkShader->setChunkLoDscale(LoDscale);
	chunkShader->setChunkSamplePos(chunk.samplePos);
	//chunkShader->setSamplesPerCube(terrain->sampleScale);

	float samplesPerCube = cubeSize / terrain->worldUnitsPerSampleUnit;
	chunkShader->setSamplesPerCube(samplesPerCube);

	
	chunkShader->setChunkTriTable(*triTableTex);
	chunkShader->setChunkTerrainPos(chunk.terrainPos);

	int vertsPerPrimitive = 3 * chunk.noAttribs;
	int maxMCverts = 16; //The maximum vertices needed for a surface inside one MC cube.
	int nVertsOut = cubesPerChunkEdge * cubesPerChunkEdge * cubesPerChunkEdge * maxMCverts;


	CBaseBuf* terrainBuf = &terrain->multiBuf;
	CBuf* srcBuf = &((CRenderModel*)shaderChunkGrid)->buf;
	unsigned int primitives = Engine.acquireFeedbackVerts(*srcBuf, drawLinesAdjacency, *tempFeedbackBuf, drawTris);




	if (primitives) {
		int outSize = primitives * 3 * sizeof(vBuf::T3DnormVert);
		totalbufsize += outSize;
		totalchunks++;

		terrainBuf->copyBuf(*tempFeedbackBuf, outSize);

		chunk.id = terrainBuf->getLastId();
		terrainBuf->setBlockColour(chunk.id, (tmpRGBAtype&)chunk.colour);

		if (chunk.LoD == 1)
			findGrassPoints(chunk);
	}
	else
		chunk.id = NULL;
	chunk.status = chSkinned;



	terrain->totalTris += (primitives * 3);
}

/** Return true if this superchunk doesn't intersect the terrain heightfield. */
//TO DO: fast but too coarse, leaves cracks
bool C3DtestApp::superChunkIsEmpty(CSuperChunk& SC) {
	
	//SCpassed++;
	//return false;


	Engine.Renderer.setShader(chunkCheckShader);
	float chunkSampleStep = SC.chunkSize / terrain->worldUnitsPerSampleUnit;
	float LoDscale = ( SC.sampleStep  ) /( cubesPerChunkEdge);
	chunkCheckShader->setSampleCorner(SC.nwSamplePos);
	chunkCheckShader->setLoDscale(LoDscale);

	unsigned int primitives = Engine.drawModelCount(*chunkShell);

	//TO DO: chunkshell is coarse, create a SCshell with more points
	if ((primitives == 0)){ // ||  (primitives == shellTotalVerts * 3)) {
		SCrejected++;
		return true; //outside surface
	}
	SCpassed++;
	SC.tmp = true;
	return false;
	
	
	
	
	
	
	
	
	
	
	
	
	
	//return false;
//	if (SC.tmpIndex.y == 7)
//		int b = 0;

	//find the SC nw corner in sample space
	vec3 SCsampleCorner = SC.nwSamplePos;
	//return false;
	//map this to the height map
	SCsampleCorner = SCsampleCorner - terrain->layers[0].nwSampleCorner ;
	float SCsampleStep = terrain->layers[terrain->layers.size() - SC.LoD].SCsampleStep;
	float scale = (terrain->worldSize.x / terrain->worldUnitsPerSampleUnit) / heightmapTex->width;
	vec3 heightMapPos = SCsampleCorner / scale;

	//read pixels
	uvec4 pixel = heightmapTex->getPixel(heightMapPos.x, heightMapPos.z);
	float height = (float)pixel.r / 255.0f; //.68
	float sampleVolumeHeight = terrain->worldSize.y / terrain->worldUnitsPerSampleUnit;
	//does the height value fall inside the vertical space of this SC?
	height = sampleVolumeHeight * height;
	//return false;
	if (height > SCsampleCorner.y   && height < (SCsampleCorner.y + SCsampleStep + 0.148f)) {
		SCpassed++;
		return false;
	}

	SCrejected++;
	return true;
}


/** Return false if no side of this potential chunk is penetratedby the isosurface.*/
bool C3DtestApp::chunkExists(vec3& sampleCorner, int LoD) {
	return true;
	//change to chunk test shader
	Engine.Renderer.setShader(chunkCheckShader);
	float LoDscale = LoD;
	//load shader values
	chunkCheckShader->setSampleCorner(sampleCorner);
	chunkCheckShader->setLoDscale(LoDscale);

	//Draw check grid 
	unsigned int primitives = Engine.drawModelCount(*chunkShell);
	if (primitives == 0)
		return false;
	if (primitives == shellTotalVerts * 3)
		return false; //outside surface
	return true;
}



void C3DtestApp::keyCheck() {
	CCamera* currentCamera = Engine.getCurrentCamera();
	float moveInc = dT * 1000.0; // 0.05125f;

	

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
		if (KeyDown[VK_SPACE]) {
			;// currentCamera->elevate(moveInc);
		}


		if (KeyDown['E']) {
			currentCamera->yaw(yawAng * dT);
		}

		if (KeyDown['T']) {
			currentCamera->yaw(-yawAng *dT);
		}

		float rot = dT * 200.0f;
		if (KeyDown['P']) {
			cube->rotate(rot, glm::vec3(1, 0, 0));
		}
		if (KeyDown['Y']) {
			cube->rotate(rot, glm::vec3(0, 1, 0));
		}
		if (KeyDown['R']) {
			//cube->rotate(rot, glm::vec3(0, 0, 1));
			chunkShader->recompile();
		
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

			if (KeyDown[VK_SPACE] && physCube == NULL) {
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

				EatKeys();

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
			setMousePos(-1, -1);

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

			setMousePos(-1, -1);
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

	if (KeyDown['1']) {
		fpsOn = !fpsOn;
		if (fpsOn) {
			//Engine.setCurrentCamera(&fpsCam);
			Engine.setCurrentCamera(&playerObject.povCam);
			playerPhys->asleep = false;
		}
		else
			Engine.setCurrentCamera(Engine.defaultCamera);
		EatKeys();

	}


	if (KeyDown['K']) {
		//A is player direction
		//B is the normal of the plane they're standing on
		// A projected onto the plane is:  B ×(A×B / |B | ) / |B |
		vec3 groundNormal = physCube->currentContactNormal;
		if (length(groundNormal) <= 0) {
			return;
		}
		vec3 eyeLine = vec3(0, 0, -1); // playerObject.povCam.getTargetDir();

		vec3 moveDir = cross(eyeLine, groundNormal) / length(groundNormal);
		moveDir = cross(groundNormal, moveDir) / length(groundNormal);

		physCube->velocity += moveDir * 0.4f;

		cerr << "\nmoveDir of " << moveDir.x << " " << moveDir.y << " " << moveDir.z;
		if (!tmp) {
			cerr << "\n!!!!Sideways push started!";
			tmp = true;
		}
		//EatKeys();
	}
	if (KeyDown['J']) {
		physCube->pModel->translate(vec3(0,0,0.05f));
		if (!tmp) {
			cerr << "\n!!!!Sideways push started!";
			tmp = true;
		}
	}
	if (KeyDown['I']) {
		physCube->pModel->translate(vec3(-0.05f,0,0));
	}
	
	if (KeyDown['M']) {
		physCube->pModel->translate(vec3(0.05f, 0, 0));
	}

	if (KeyDown['N']) {
		vec3 camPos = physCube->position;
		cerr << "\ncube pos " << camPos.x << " " << camPos.y << " " << camPos.z;
		
		EatKeys();
	}

	if (physCube != NULL)
		if  (KeyDown[VK_SPACE]) {
		physCube->velocity.y += 20;
		physCube->velocity.z *= 2;
		physCube->velocity.x *= 2;

		cerr << "\nJumping!!!";
		EatKeys();
	}




	if (KeyDown['Z']) {
		advance(north);
		advance(west);
		EatKeys();
	}

	if (KeyDown['X']) {
		terrain->clear();
		chunkShader->recompile();
		terrain2texShader->recompile();
		terrain->createAllChunks();
		updateHeightmapImage();

		EatKeys();
	}

	if (KeyDown['C']) {
	/*	vec3 pos = currentCamera->getPos();
		pos = pos + currentCamera->getTargetDir() * 200.0f;
		CModel* cube = Engine.createCube(pos, vec3(40));
		physCube = Engine.addPhysics(cube);
		physCube->setMass(10);
		physCube->bSphere.setRadius(35);
		physCube->AABB.setSize(40, 40);*/
		
		terrain->createAllChunks();

		EatKeys();
	}

	if (KeyDown['V']) {
	

		vec3 pos = currentCamera->getPos();
		//pos = vec3(293.96, 198.179, -82.5066);
		
		pos = pos + currentCamera->getTargetDir() * 30.0f;
		//pos = vec3(-3.6289, 335.689, -8.32864);
		CModel* cube = Engine.createCube(pos, vec3(5));
		Engine.modelDrawList.push_back(cube);
		physCube = Engine.addPhysics(cube);
		physCube->setMass(10);
		physCube->bSphere.setRadius(2);
		physCube->AABB.setSize(5, 5);
	

		EatKeys();
	}

	if (KeyDown['B']) {
		vec3 camPos = currentCamera->getPos();
		//camPos = physCube->getModel()->getPos();
		cerr << "\ncam pos " << camPos.x << " " << camPos.y << " " << camPos.z;
		vec3 camTarg = currentCamera->getTargetDir();
		cerr << "\ncam target " << camTarg.x << " " << camTarg.y << " " << camTarg.z;
		

		EatKeys();
	}


	//if (keyNow('U') )
	if (KeyDown['U']) {
		supWire = !supWire;
		EatKeys();
	}

	if (KeyDown['H']) {
		heightmapImage->visible = !heightmapImage->visible;
		if (heightmapImage->visible)
			updateHeightmapImage();
		EatKeys();
	}

	selectChk = glm::mod(vec3(selectChk), vec3(15, 5, 15));

}

void C3DtestApp::OnKeyDown(unsigned int wParam, long lParam) {



};


/** Called when mouse moves. */
void C3DtestApp::mouseMove(int x, int y, int key) {
	return;

}


void C3DtestApp::onResize(int width, int height) {

}



void C3DtestApp::draw() {

	
	//return;
	//draw chunk
	glm::mat3 normMatrix(terrain->worldMatrix);

	mat4 mvp;
	CCamera* currentCamera = Engine.getCurrentCamera();

	int draw = 0;
	double t = Engine.Time.milliseconds();

	Engine.Renderer.setShader(Engine.phongShader);
//	

	mvp = currentCamera->clipMatrix * terrain->chunkOrigin;


	mat3 tmp;
	Engine.phongShader->setMVP(mvp);
	Engine.phongShader->setNormalModelToCameraMatrix(tmp); //why am I doing this?

	terrain->drawNew();

	Engine.Renderer.setShader(grassShader);
	drawGrass(mvp);

	t = Engine.Time.milliseconds() - t;

/*	CBuf* buf = (CBuf*) dummy2;
	grassShader->setMVP(mvp);
	Engine.Renderer.backFaceCulling(false);
	if (dummy2)
		Engine.Renderer.drawBuf(*buf, drawPoints);
	Engine.Renderer.backFaceCulling(true);
	*/

	//wireframe drawing:
//	Engine.Renderer.setShader(Engine.wireShader);
	//Engine.wireShader->setColour(vec4(0, 1, 0, 0.4f));

	//draw bounding boxes
	if (supWire) {
		int component = 1;
		CSuperChunk* sc;
		vec4 colour = vec4(0, 0, 0, 1);
		vec3 cornerAdjust, opCornerAdjust;
		Engine.Renderer.setShader(Engine.wireBoxShader);
		for (int l = terrain->layers.size()-1; l > -1; l--) {
			vBuf::T3DnormVert box[2000]; //should be enough
			int index = 0;
			for (int s = 0; s < terrain->layers[l].superChunks.size(); s++) {
				sc = terrain->layers[l].superChunks[s];
				//if (sc->tmp) 
				{
					box[index].v = sc->nwWorldPos + terrain->layers[l].nwLayerPos;
					cornerAdjust = vec3(sc->faceBoundary[west], sc->faceBoundary[down], sc->faceBoundary[north]);
					box[index].v += cornerAdjust * sc->chunkSize;

					opCornerAdjust = vec3(sc->faceBoundary[east] + 1, sc->faceBoundary[up] + 1,
						sc->faceBoundary[south] + 1);
					opCornerAdjust -= cornerAdjust;
					box[index].normal = opCornerAdjust * sc->chunkSize;
					index++;
				}
			}
			wireSCs->storeVertexes(box, sizeof(vBuf::T3DnormVert) * index, index);
			wireSCs->storeLayout(3, 3, 0, 0);
			mvp = currentCamera->clipMatrix * wireSCs->worldMatrix;
			Engine.wireBoxShader->setMVP(mvp);
			
			colour[component++] = 0.8f;
			if (component > 2)
				component = 0;
			Engine.wireBoxShader->setColour(colour);
			wireSCs->drawNew();
		}
		
	
	}

	if (!fpsOn) {
		Engine.Renderer.setShader(Engine.phongShader);
		mvp = currentCamera->clipMatrix * playerObject.pModel->worldMatrix;
		Engine.phongShader->setMVP(mvp);
		playerObject.pModel->drawNew();
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
	vec3 movement = dir *  float(10.0f);  //was 1
	//terrain->translate(movement);
	//vec3 pos = terrain->getPos();
	//terrain->chunkOrigin[3] += vec4(movement, 0);

	terrain->scrollTriggerPoint += vec3(movement);
	vec3 pos = terrain->scrollTriggerPoint;


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
		terrain->chunkOriginInt += dir;

		terrain->scrollTriggerPoint = pos;
		vec3 translation =  vec3(terrain->chunkOriginInt *  cubesPerChunkEdge) * cubeSize ;
		terrain->chunkOrigin[3] = vec4(translation, 1);
		terrain->advance(direction); 
		onTerrainAdvance(direction);
	}
}

/** Called every frame. Mainly use this to scroll terrain if we're moving in first-person mode*/
void C3DtestApp::Update() {
	//float move = dT * 0.00005;

	if (skyDome)
		skyDome->update(dT);

	terrain->update();

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

			vec3 translation = vec3(terrain->chunkOriginInt *  cubesPerChunkEdge) * cubeSize;
			//work out direction to scroll-in new terrain from
			if (outsideChunkBoundary.x) {
				if (pos.x > 0)
					direction = east;
				else
					direction = west;
				terrain->chunkOriginInt += dirToVec(flipDir(direction));
				terrain->chunkOrigin[3] = vec4(chunkDist * vec3(terrain->chunkOriginInt), 1);
				terrain->advance(direction);
				//return;
			}

			if (outsideChunkBoundary.y) {
				if (pos.y > 0)
					direction = up;
				else
					direction = down;
				terrain->chunkOriginInt += dirToVec(flipDir(direction));
				terrain->chunkOrigin[3] = vec4(chunkDist * vec3(terrain->chunkOriginInt), 1);
				terrain->advance(direction);
				//return;
			}


			if (outsideChunkBoundary.z) {
				if (pos.z > 0)
					direction = south;
				else
					direction = north;
				terrain->chunkOriginInt += dirToVec(flipDir(direction));
				terrain->chunkOrigin[3] = vec4(chunkDist * vec3(terrain->chunkOriginInt) , 1);
				terrain->advance(direction);
			}
		}
	}
}

/** Prepare a hollow shell of vertices to use in checks for empty chunks. */
void C3DtestApp::initChunkShell() {
	float vertsPerEdge = cubesPerChunkEdge + 1;
	shellTotalVerts = std::pow(vertsPerEdge, 3) - std::pow(vertsPerEdge - 2, 3);
	vec3* shell = new vec3[shellTotalVerts];
	int v = 0;
	for (int y = 0; y < vertsPerEdge; y++) {
		for (int x = 0; x < vertsPerEdge; x++) {
			shell[v++] = vec3(x, y, 0);
			shell[v++] = vec3(x, y, cubesPerChunkEdge);
		}
		for (int z = 1; z < cubesPerChunkEdge; z++) {
			shell[v++] = vec3(0, y, z);
			shell[v++] = vec3(cubesPerChunkEdge, y, z);
		}
	}

	for (int x = 1; x < cubesPerChunkEdge; x++) {
		for (int z = 1; z < cubesPerChunkEdge; z++) {
			shell[v++] = vec3(x, 0,z);
			shell[v++] = vec3(x, cubesPerChunkEdge,z);
		}
	}

	chunkShell = Engine.createModel();
	chunkShell->setDrawMode( drawPoints);
	chunkShell->storeVertexes(shell, sizeof(vec3) * v, v);
	chunkShell->storeLayout(3, 0, 0, 0);
	delete[] shell;
	chunkShell->getMaterial()->setShader(chunkCheckShader);
}

/** Initialise a 3D grid of points to represent the cubes of a chunk in drawing. */
void C3DtestApp::initChunkGrid(int cubesPerChunkEdge) {
	int vertsPerEdge = cubesPerChunkEdge + 1;
	int noVerts = vertsPerEdge * vertsPerEdge * vertsPerEdge;
	vec3* shaderChunkVerts = new vec3[noVerts];
	int i = 0;
	for (float y = 0; y < vertsPerEdge; y++) {
		for (float z = 0; z < vertsPerEdge; z++) {
			for (float x = 0; x < vertsPerEdge; x++) {
				shaderChunkVerts[i++] = vec3(x, y, z);
			}
		}
	}


	//create an index that creates a bottom square for each cube in the grid
	int noIndices = cubesPerChunkEdge * cubesPerChunkEdge *cubesPerChunkEdge * 5;

	unsigned short layer = vertsPerEdge * vertsPerEdge;
	unsigned short* index = new unsigned short[noIndices];
	i = 0;
	unsigned short vertNo = 0;
	do {
		index[i++] = vertNo;
		index[i++] = vertNo + 1;
		index[i++] = vertNo + 1 + layer;
		index[i++] = vertNo + layer;


		index[i++] = 65535; //signals the end of this line sequence

		vertNo++;
		if (((vertNo + 1) % vertsPerEdge) == 0)
			vertNo++;
		if ((vertNo % layer) == (vertsPerEdge * (vertsPerEdge - 1)))
			vertNo += vertsPerEdge;

	} while (i < noIndices);

	shaderChunkGrid = Engine.createModel();

	shaderChunkGrid->setDrawMode(drawLinesAdjacency);

	//Engine.(&shaderChunkGrid,shaderChunkVerts,index);
	//Engine.setVertexDetails(shaderChunkGrid, 1, noIndices, noVerts);
	//Engine.storeIndexedModel(shaderChunkGrid,shaderChunkVerts,noVerts, index);

	shaderChunkGrid->storeVertexes(shaderChunkVerts, sizeof(vec3) * noVerts, noVerts);
	shaderChunkGrid->storeIndex(index, sizeof(unsigned short)*noIndices, noIndices);
	shaderChunkGrid->storeLayout(3, 0, 0, 0);

	delete[] shaderChunkVerts;
	delete[] index;
}

/** Create a model for drawing superChunk positions in wireframe. */
void C3DtestApp::initWireSCs() {
	wireSCs = Engine.createModel();
	
	wireSCs->drawMode = GL_POINTS;
	wireSCs->getMaterial()->setShader(Engine.wireBoxShader);

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
	
	
	heightmapImage->visible = false;

	//set up shader(s)
	terrain2texShader = new CTerrain2texShader();
	terrain2texShader->pRenderer = &Engine.Renderer;
	terrain2texShader->create(dataPath + "terrain2tex");
	terrain2texShader->getShaderHandles();
	terrain2texShader->setType(userShader);
	Engine.shaderList.push_back(terrain2texShader);
}

/** Render the current terrain shader to our heightmap image. */
void C3DtestApp::updateHeightmapImage() {
	//activate the render-to-texture shader

	heightmapImage->setTexture(*heightmapTex);


	heightmapImage->setTexture(*heightmapTex);
	Engine.Renderer.setShader(terrain2texShader);
	vec3 sampleCorner = terrain->layers[0].nwSampleCorner;
	terrain2texShader->setNwSampleCorner(vec2(sampleCorner.x,sampleCorner.z));

	//calculate scaling
	float scale = (terrain->worldSize.x / terrain->worldUnitsPerSampleUnit) / heightmapTex->width;

	terrain2texShader->setPixelScale(scale);

	Engine.Renderer.renderToTextureQuad(*heightmapTex);

	
}

/** Initialise shader and buffer required to run a terrain height-finding query. */
void C3DtestApp::initHeightFinder() {
	terrainPointShader = new CTerrainPointShader();
	terrainPointShader->feedbackVaryings[0] = "result";
	Engine.shaderList.push_back(terrainPointShader);
	terrainPointShader->pRenderer = &Engine.Renderer;
	terrainPointShader->load(vertex,dataPath + "terrainPoint.vert");
	terrainPointShader->attach();
	terrainPointShader->setFeedbackData(1);
	terrainPointShader->link();
	terrainPointShader->getShaderHandles();

	//create verts
	vec3* v = new vec3[findHeightVerts];
	for (int x = 0; x < findHeightVerts; x++)
		v[x] = vec3(0, x, 0);

	heightFinderBuf.storeVertexes(v, sizeof(vec3) * findHeightVerts, findHeightVerts);
	heightFinderBuf.storeLayout(3, 0, 0, 0);
	delete v;
}

/** Generate a landscape centred on the given point. */
void C3DtestApp::createTerrain(glm::vec2 & centre) {
	//find the terrain height at this point
	const float defaultStart = terrain->worldUnitsPerSampleUnit / 1000;
	float height = findTerrainHeight(vec3(centre.x, -defaultStart ,centre.y));
	terrain->setSampleCentre(vec3(centre.x, height, centre.y));
}

/** Searching up from the given point in sample space, return the height at which terrain is first encountered. */
float C3DtestApp::findTerrainHeight(glm::vec3& basePos) {
	Engine.Renderer.setShader(terrainPointShader);
	float offsetScale = 1 / terrain->worldUnitsPerSampleUnit ;
	terrainPointShader->setOffsetScale(offsetScale);
	vec3 startPos = basePos;
	CBaseBuf* heightResultsBuf = Engine.createBuffer(); 
	heightResultsBuf->setSize(sizeof(float) * findHeightVerts);

	float* heightResults = new float[findHeightVerts];
	float terrainHeight = 0;; const float MCvertexTest = 0.5f;
	
	for (int step = 0; step < 100; step++) {
		terrainPointShader->setSampleBase(startPos);
		Engine.acquireFeedbackVerts(heightFinderBuf, drawPoints, *heightResultsBuf, drawPoints);
		heightResultsBuf->getData((unsigned char*)heightResults, sizeof(float) * findHeightVerts);
		for (int r = 0; r < findHeightVerts; r++) {
			if (heightResults[r] < MCvertexTest) 
				terrainHeight = startPos.y + (r * offsetScale);
		}
		startPos.y += findHeightVerts * offsetScale;
	}
	delete heightResults;
	return terrainHeight;
}

/** Do the necessary setup for finding grass placement points on chunks. */
void C3DtestApp::initGrassFinding() {
	//Create a buffer of evenly distributed random points for grass placement.
	grassPoints = Engine.createBuffer();
	std::vector <glm::vec2> points2D;
	float LoD1chunkSize = terrain->LoD1cubeSize * cubesPerChunkEdge;
	points2D = pois::generate_poisson(LoD1chunkSize, LoD1chunkSize, 0.8f, 10);
	noGrassPoints = points2D.size();
	std::vector < glm::vec3> points3D(noGrassPoints);
	for (int p = 0; p < noGrassPoints; p++) {
		points3D[p].x = points2D[p].x;
		points3D[p].z = points2D[p].y;
		points3D[p].y = -FLT_MAX;
	}
	grassPoints->storeVertexes(points3D.data(), sizeof(glm::vec3) * noGrassPoints, noGrassPoints);
	grassPoints->storeLayout(3, 0, 0, 0);

	//load the point finding shader
	findPointHeightShader = new CFindPointHeightShader();
	findPointHeightShader->feedbackVaryings[0] = "newPoint";
	Engine.shaderList.push_back(findPointHeightShader);
	findPointHeightShader->pRenderer = &Engine.Renderer;
	findPointHeightShader->load(vertex, dataPath + "findPointHeight.vert");
	findPointHeightShader->attach();
	findPointHeightShader->setFeedbackData(1);
	findPointHeightShader->link();
	findPointHeightShader->getShaderHandles();


	CBaseBuf* dummy = Engine.createBuffer();
	vec3 v(1);
	unsigned short index = 0;
	dummy->storeVertexes(&v, sizeof(vec3), 1);
	dummy->storeIndex(&index, sizeof(index), 1);
	dummy->storeLayout(3, 0, 0, 0);

	terrain->grassMultiBuf.setSize(grassBufSize);
	terrain->grassMultiBuf.setInstanced(*dummy, 1);
	terrain->grassMultiBuf.storeLayout(3, 3, 0, 0);

	CBaseTexture* grassTex = Engine.Renderer.textureManager.getTexture(dataPath + "grassPack.dds");

	//load the grass drawing shader
	grassShader = new CGrassShader();
	grassShader->pRenderer = &Engine.Renderer;
	grassShader->create(dataPath + "grass");
	grassShader->getShaderHandles();
	grassShader->setType(userShader);
	Engine.shaderList.push_back(grassShader);
}

/**	Create a selection of points on the terrain surface of this chunk where grass
	can be drawn. */
void C3DtestApp::findGrassPoints(Chunk & chunk) {
	float chunkSize = terrain->LoD1cubeSize * terrain->cubesPerChunkEdge;
	//load shader
	Engine.Renderer.setShader(findPointHeightShader);
	findPointHeightShader->setCurrentY(0);
	findPointHeightShader->setSamplePosition(chunk.samplePos);
	findPointHeightShader->setSampleScale(1.0f / terrain->worldUnitsPerSampleUnit);
	findPointHeightShader->setChunkLocaliser(glm::vec3(0));

	//copy grasspoints
	CBaseBuf* pointBuf = Engine.createBuffer();
	pointBuf->setSize(noGrassPoints * sizeof(glm::vec3));
	pointBuf->copyBuf(*grassPoints, noGrassPoints * sizeof(glm::vec3));


	CBaseBuf* outBuf = Engine.createBuffer();
	outBuf->setSize(noGrassPoints * sizeof(glm::vec3));
	outBuf->setNoVerts(noGrassPoints);
	outBuf->storeLayout(3, 0, 0, 0);
	
	//draw chunk using transform feedback
	int points = Engine.acquireFeedbackVerts(*pointBuf, drawPoints, *outBuf, drawPoints);

	//loop
	CBaseBuf* srcBuf = outBuf; CBaseBuf* destBuf = pointBuf; CBaseBuf* swapBuf;
	float stepHeight = chunkSize / 16;
	for (int step = 1; step <= 16; step++) {
		if (step == 16)
			findPointHeightShader->setChunkLocaliser(chunk.terrainPos);
		findPointHeightShader->setCurrentY( step * stepHeight);
		int noPrimitives = Engine.acquireFeedbackVerts(*srcBuf, drawPoints, *destBuf, drawPoints);
		//feed points back into shader
		swapBuf = srcBuf;
		srcBuf = destBuf;
		destBuf = swapBuf;
	}



	terrain->grassMultiBuf.copyBuf(*srcBuf, srcBuf->getBufSize());
	chunk.grassId = terrain->grassMultiBuf.getLastId();
}


/** Run through the grass multibuf, drawing instanced grass. */
void C3DtestApp::drawGrass(glm::mat4& mvp) {
	Engine.Renderer.backFaceCulling(false);
	Engine.Renderer.setShader(grassShader);
	grassShader->setMVP(mvp);

	CChildBuf* childBuf;
	for (int child = 0; child < terrain->grassMultiBuf.noChildBufs; child++) {
		childBuf = &terrain->grassMultiBuf.childBufs[child];

		unsigned int nInstancedVerts = childBuf->instancedBuf->getNoVerts();
		unsigned int nIndices = childBuf->instancedBuf->getNoIndices();
		glBindVertexArray(terrain->grassMultiBuf.childBufs[child].hVAO);
		for (int object = 0; object < childBuf->objCount; object++) {
			glDrawElementsInstancedBaseInstance(GL_POINTS, nIndices, GL_UNSIGNED_SHORT, 0,
				childBuf->count[object], childBuf->first[object]);
		}
	}
	Engine.Renderer.backFaceCulling(true);
}



C3DtestApp::~C3DtestApp() {
	delete chunkCheckShader;
	delete terrain;
	//TO DO: since these are created with Engine.createModel, engine should handle deletion.
	//delete chunkShell;
//	delete shaderChunkGrid;
	//delete chunkBB;
//	delete tempFeedbackBuf;
}





