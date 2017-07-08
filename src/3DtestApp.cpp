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

using namespace watch;

using namespace glm;



C3DtestApp::C3DtestApp() {

	tmpSCno = 0;
	tmp = false;
	physCube = NULL;
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

	Engine.getCurrentCamera()->setPos(vec3(83.8443, 259.706, 34.8063));
	Engine.getCurrentCamera()->setPos(vec3(83.8443, 359.706, 34.8063));
	Engine.getCurrentCamera()->lookAt(vec3(-0.995789, -0.0588613, 0.070279));


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
	tempFeedbackBuf->setSize(500000);



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

	double t = Engine.Time.milliseconds();

	
	initChunkGrid(cubesPerChunkEdge);
	//4 16 2.5
	terrain->setSizes(chunksPerSuperChunkEdge, cubesPerChunkEdge, cubeSize);

	terrain->createLayers2(5120, 320, 2); //1280 320
	//terrain->createLayers2(1280, 320, 0);

	//terrain->createLayers(8, 2, 2); //(8, 3, 2); //(4,2,1);

	terrain->createAllChunks();

	t = Engine.Time.milliseconds() - t;
	cerr << "\n time " << t;

	


	//load chunk shader
	const char* feedbackVaryings[23];
	feedbackVaryings[0] = "gl_Position";
	feedbackVaryings[1] = "normal";

	chunkShader = new ChunkShader();
	Engine.shaderList.push_back(chunkShader);
	chunkShader->pRenderer = &Engine.Renderer;
	chunkShader->load(vertex, dataPath + "chunk.vert");
	chunkShader->load(geometry, dataPath + "chunk.geom");
	chunkShader->attach();
	chunkShader->setFeedbackData(2, feedbackVaryings);
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
	playerObject.pModel = Engine.createCube(vec3(0), vec3(playerObject.width, playerObject.height, playerObject.width));
	playerObject.setPos(vec3(0, 237, 0));
	playerObject.setPos(vec3(0, 50, 0));

	playerPhys = Engine.addPhysics(&playerObject);
	playerPhys->setMass(10);
	playerPhys->AABB.setSize(1, 1);
	playerPhys->asleep = true;
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
	chunkShader->setSamplesPerCube(terrain->sampleScale);
	chunkShader->setChunkTriTable(*triTableTex);

	chunkShader->setChunkTerrainPos(chunk.terrainPos);

	int vertsPerPrimitive = 3 * chunk.noAttribs;
	int maxMCverts = 16; //The maximum vertices needed for a surface inside one MC cube.
	int nVertsOut = cubesPerChunkEdge * cubesPerChunkEdge * cubesPerChunkEdge * maxMCverts;

	CBaseBuf* terrainBuf = &terrain->multiBuf;

	unsigned int primitives = Engine.acquireFeedbackVerts(*shaderChunkGrid, *tempFeedbackBuf, *terrainBuf);

	if (primitives) {
		chunk.id = terrainBuf->getLastId();
		terrainBuf->setBlockColour(chunk.id, (tmpRGBAtype&)chunk.colour);
	}
	else
		chunk.id = NULL;
	chunk.status = chSkinned;



	terrain->totalTris += (primitives * 3);
}


bool C3DtestApp::superChunkIsEmpty(vec3& sampleCorner, int LoD) {
	Engine.Renderer.setShader(chunkCheckShader);
	float LoDscale = LoD * chunksPerSuperChunkEdge;
	chunkCheckShader->setSampleCorner(sampleCorner);
	chunkCheckShader->setLoDscale(LoDscale);

	unsigned int primitives = Engine.drawModelCount(*chunkShell);

	//TO DO: chunkshell is coarse, create a SCshell with more points
	if ((primitives == 0) || (primitives == shellTotalVerts * 3))
		return true; //outside surface
	return false;
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
	float moveInc = dT * 500.0; // 0.05125f;

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
			cube->rotate(rot, glm::vec3(0, 0, 1));
		}

	}
	else {
		vec3 moveDir(0);
		std::cerr << "\nvertical velocity at keypress: " << playerPhys->velocity.y;

		if (length(playerPhys->currentContactNormal) <= 0) {
			cerr << "\nmove aborted";
			return;
		}


			vec3 dir = playerObject.povCam.getTargetDir();
			vec3 groundNormal = playerPhys->currentContactNormal;
			vec3 eyeLine = playerObject.povCam.getTargetDir();
			vec3 flip;
			
			if (KeyDown[VK_SPACE] && physCube == NULL) {
				playerPhys->velocity.y += 20;
				playerPhys->velocity.x *= 2.5f;
				playerPhys->velocity.z *= 2.5f;
				
				cerr << "\nJumping!!!";
			}

		if (keyNow('W')) {
			moveDir = cross(eyeLine, groundNormal) / length(groundNormal);
			moveDir = cross(groundNormal, moveDir) / length(groundNormal);

				playerPhys->velocity += moveDir * 1.4f;   //0.03f safe but slow //0.2f formerly caused bounces

	
				cerr << "\n********Moving!";
				cerr << "\nMoveDir " << moveDir.x << " " << moveDir.y << " " << moveDir.z;
				cerr << " Speed of " << length(playerPhys->velocity);
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
	if (KeyDown['5']) {
		advance(up);
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
		for (int l = 0; l < terrain->layers.size(); l++) {
			for (int sc = 0; sc < terrain->layers[l].superChunks.size(); sc++) {
				terrain->layers[l].superChunks[sc]->removeAllChunks();
			}
		}
		//	terrain->layers[0].superChunks[149]->removeAllChunks();

				//dbgSC->removeAllChunks();
		EatKeys();
	}

	if (KeyDown['C']) {
		vec3 pos = currentCamera->getPos();
		pos = pos + currentCamera->getTargetDir() * 200.0f;
		CModel* cube = Engine.createCube(pos, vec3(40));
		physCube = Engine.addPhysics(cube);
		physCube->setMass(10);
		physCube->bSphere.setRadius(35);
		physCube->AABB.setSize(40, 40);
		

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

	mvp = currentCamera->clipMatrix * terrain->chunkOrigin;
	mat3 tmp;
	Engine.phongShader->setMVP(mvp);
	Engine.phongShader->setNormalModelToCameraMatrix(tmp); //why am I doing this?

	terrain->drawNew();

	t = Engine.Time.milliseconds() - t;


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
			vBuf::T3DnormVert box[500]; //should be enough
			int index = 0;
			for (int s = 0; s < terrain->layers[l].superChunks.size(); s++) {
				sc = terrain->layers[l].superChunks[s];
				box[index].v = sc->nwWorldPos + terrain->layers[l].nwLayerPos;
				cornerAdjust = vec3(sc->faceBoundary[west], sc->faceBoundary[down], sc->faceBoundary[north]);
				box[index].v += cornerAdjust * sc->chunkSize;

				opCornerAdjust = vec3(sc->faceBoundary[east] + 1, sc->faceBoundary[up] + 1,
					sc->faceBoundary[south] + 1);
				opCornerAdjust -= cornerAdjust;
				box[index].normal = opCornerAdjust * sc->chunkSize;
				index++;
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
	CSuperChunk* sc = terrain->getSC(pos);

	if (sc)
		watch1 << sc->tmpIndex.x << " " << sc->tmpIndex.y << " " << sc->tmpIndex.z << " ";

	if (fpsOn) {

	//	if (terrain->toSkin.size() != 0)
	//		return;
		//cheap dirty fix for the problem of scrolling in one direction before we've finished scrolling in another

		Tdirection direction = none;

		float chunkDist = cubesPerChunkEdge * cubeSize;

		vec3 pos;
	//	pos = fpsCam.getPos();
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
			shell[v++] = vec3(0, z, y);
			shell[v++] = vec3(cubesPerChunkEdge, z, y);
		}
	}

	for (int x = 1; x < cubesPerChunkEdge; x++) {
		for (int z = 1; z < cubesPerChunkEdge; z++) {
			shell[v++] = vec3(x, z, 0);
			shell[v++] = vec3(x, z, cubesPerChunkEdge);
		}
	}

	chunkShell = Engine.createModel();
	chunkShell->drawMode = GL_POINTS;
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

	shaderChunkGrid->drawMode = GL_LINES_ADJACENCY;

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



C3DtestApp::~C3DtestApp() {
	delete chunkCheckShader;
	delete terrain;
	//TO DO: since these are created with Engine.createModel, engine should handle deletion.
	//delete chunkShell;
//	delete shaderChunkGrid;
	//delete chunkBB;
//	delete tempFeedbackBuf;
}





