#include "3DtestApp.h"

#include <glew.h>
#include <algorithm>
#include <string>
#include <vector>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/common.hpp>

#include <glm/gtc/matrix_access.hpp>	 //temp

#include "watch.h"

using namespace watch;

using namespace glm;



C3DtestApp::C3DtestApp() {

	tmpSCno = 0;

}

void C3DtestApp::onStart() {


	chunkCall = 0;

	dataPath = homeDir + "Data\\";
	lastMousePos = glm::vec2(0, 0);

	//test objects, temporary
	cube = Engine.createCube(vec3(-3, 300, -3), 1.0f);
	Engine.createCube(vec3(3, 300, -3), 1.0f);
	Engine.createCylinder(vec3(0, 300, -4), 1, 2, 30);


	//position the default camera
	Engine.getCurrentCamera()->setPos(vec3(0, 303, 6));
	Engine.getCurrentCamera()->lookAt(vec3(0, -1, -3));

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
	//terrain->setSizes(4, 18, cubeSize);
	terrain->createLayers(4, 2, 1); //(8, 3, 2); //(4,2,1);

	terrain->createAllChunks();

	t = Engine.Time.milliseconds() - t;
	cerr << "\n time " << t;


	createBB();

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

	oldTime = Engine.Time.milliseconds();

	supWire = false;

}

/** Create a wireframe bounding box.*/
void C3DtestApp::createBB() {
	chunkBB = Engine.createModel();
	chunkBB->drawMode = GL_LINES;
	vec3 boxV[8] = { vec3(0,0,0),
							vec3(1,0,0),
							vec3(1,1,0),
							vec3(0,1,0),
							vec3(0,0,1),
							vec3(1,0,1),
							vec3(1,1,1),
							vec3(0,1,1) };
	unsigned short index[12 * 2] = { 0,1,1,2,2,3,3,0,
									4,5,5,6,6,7,7,4,
									6,2,7,3,5,1,4,0 };

	chunkBB->storeVertexes(boxV, sizeof(boxV), 8);
	chunkBB->storeIndex(index, sizeof(index), 24);
	chunkBB->storeLayout(3, 0, 0, 0);
	chunkBB->getMaterial()->setShader(Engine.wireShader);
}


/*  Create a mesh for this chunk, and register it with the renderer.  */
void C3DtestApp::createChunkMesh(Chunk& chunk) {
	chunkCall++;

	//Engine.setCurrentShader(hChunkProg);
	Engine.Renderer.setShader(chunkShader);
	//Engine.setShaderValue(hChunkCubeSize,chunk.cubeSize);
	chunkShader->setChunkCubeSize(chunk.cubeSize);


	float LoDscale = float(1 << chunk.LoD - 1);
	//Engine.setShaderValue(hChunkLoDscale,LoDscale);
	chunkShader->setChunkLoDscale(LoDscale);
	//Engine.setShaderValue(hChunkSamplePos,chunk.samplePos);
	chunkShader->setChunkSamplePos(chunk.samplePos);
	//Engine.setShaderValue(hSamplesPerCube, terrain->sampleScale);
	chunkShader->setSamplesPerCube(terrain->sampleScale);
	//Engine.setDataTexture(hTriTableTex);
	chunkShader->setChunkTriTable(*triTableTex);

	//Engine.setShaderValue(hChunkTerrainPos, chunk.terrainPos);
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
	float moveInc = dT * 0.5; // 0.05125f;

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
		currentCamera->elevate(moveInc);
	}


	if (KeyDown['E']) {
		currentCamera->yaw(yawAng * dT);
	}

	if (KeyDown['T']) {
		currentCamera->yaw(-yawAng *dT);
	}

	float rot = dT * 0.2f;
	if (KeyDown['P']) {
		cube->rotate(rot, glm::vec3(1, 0, 0));
	}
	if (KeyDown['Y']) {
		cube->rotate(rot, glm::vec3(0, 1, 0));
	}
	if (KeyDown['R']) {
		cube->rotate(rot, glm::vec3(0, 0, 1));
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
			Engine.setCurrentCamera(&fpsCam);
		}
		else
			Engine.setCurrentCamera(Engine.defaultCamera);
		EatKeys();

	}


	if (KeyDown['J']) {
		selectChk.x--;
		EatKeys();
	}
	if (KeyDown['L']) {
		selectChk.x++;
		EatKeys();
	}
	if (KeyDown['I']) {
		selectChk.z--;
		EatKeys();
	}
	if (KeyDown['K']) {
		selectChk.z++;
		EatKeys();
	}
	if (KeyDown['H']) {
		selectChk.y++;
		EatKeys();
	}
	if (KeyDown['N']) {
		selectChk.y--;
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
	Engine.Renderer.setShader(Engine.wireShader);
	Engine.wireShader->setColour(vec4(0, 1, 0, 0.4f));

	//draw bounding boxes
	if (supWire) {
		//draw superchunk
		float siz;

		for (int l = 0; l < terrain->layers.size(); l++) {
			siz = terrain->layers[l].cubeSize * cubesPerChunkEdge * chunksPerSuperChunkEdge;
			Engine.wireShader->setScale(vec3(siz));
			for (int s = 0; s < terrain->layers[l].superChunks.size(); s++) {
				Engine.Renderer.setShader(Engine.wireShader);
				chunkBB->setPos(terrain->layers[l].superChunks[s]->nwWorldPos);
				mvp = currentCamera->clipMatrix * chunkBB->worldMatrix;
				Engine.wireShader->setMVP(mvp);
				chunkBB->drawNew();
			}
		}
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
	vec3 movement = dir *  float(1.0f);  //was 1
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
		terrain->advance(direction); //
	}
}

/** Called every frame. Mainly use this to scroll terrain if we're moving in first-person mode*/
void C3DtestApp::Update() {
	//float move = dT * 0.00005;

	if (skyDome)
		skyDome->update(dT);

	terrain->update();

	

	if (fpsOn) {

		if (terrain->toSkin.size() != 0)
			return;
		//cheap dirty fix for the problem of scrolling in one direction before we've finished scrolling in another

		Tdirection direction = none;

		float chunkDist = cubesPerChunkEdge * cubeSize;

		vec3 pos = fpsCam.getPos();
		bvec3 outsideChunkBoundary = glm::greaterThan(glm::abs(pos), vec3(chunkDist));


		//has viewpoint moved beyond the length of one chunk?
		if (outsideChunkBoundary.x || outsideChunkBoundary.y || outsideChunkBoundary.z) {
			vec3 posMod;
			posMod.x = fmod(pos.x, chunkDist);
			posMod.y = fmod(pos.y, chunkDist);  //was pos.y
			posMod.z = fmod(pos.z, chunkDist);

			//	if (outsideChunkBoundary.x)
			//		posMod.x = 0;
			//	if (outsideChunkBoundary.z)
			//		posMod.z = 0;


			fpsCam.setPos(posMod); //secretly reposition viewpoint prior to scrolling terrain


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



C3DtestApp::~C3DtestApp() {
	delete chunkCheckShader;
	delete terrain;
	//TO DO: since these are created with Engine.createModel, engine should handle deletion.
	//delete chunkShell;
//	delete shaderChunkGrid;
	//delete chunkBB;
//	delete tempFeedbackBuf;
}





