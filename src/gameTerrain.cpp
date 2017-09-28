#include "gameTerrain.h"

#include "renderer\renderMaterial.h"

#include "poisson.h"

using namespace glm;

CGameTerrain::CGameTerrain() {
	//load chunkCheck shader
	chunkCheckShader = new ChunkCheckShader();
	chunkCheckShader->create(pRenderer->dataPath + "chunkCheck");
	chunkCheckShader->getShaderHandles();

	tempFeedbackBuf = pRenderer->createBuffer();
	tempFeedbackBuf->setSize(1000000);
}

/** Prepare a hollow shell of vertices to use in checks for empty chunks. */
void CGameTerrain::initChunkShell() {
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
			shell[v++] = vec3(x, 0, z);
			shell[v++] = vec3(x, cubesPerChunkEdge, z);
		}
	}

	chunkShell = new CRenderModel();

	CRenderMaterial* material = new CRenderMaterial();
	chunkShell->setMaterial(*material);
	chunkShell->setDrawMode(drawPoints);
	chunkShell->storeVertexes(shell, sizeof(vec3), v);
	chunkShell->storeLayout(3, 0, 0, 0);
	delete[] shell;
	chunkShell->getMaterial()->setShader(chunkCheckShader);
}

/** Initialise a 3D grid of points to represent the cubes of a chunk in drawing. */
void CGameTerrain::initChunkGrid(int cubesPerChunkEdge) {
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
	unsigned int* index = new unsigned int[noIndices];
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

	//shaderChunkGrid = Engine.createModel();
	{shaderChunkGrid = new CRenderModel();
	//CMaterial* material = pRenderer. createMaterial();

	CRenderMaterial* material = new CRenderMaterial();
	material->setShader(pRenderer->phongShader);
	//materialList.push_back(material);
	shaderChunkGrid->setMaterial(*material);
	//modelList.push_back(model); 
	}

	shaderChunkGrid->setDrawMode(drawLinesAdjacency);

	//Engine.(&shaderChunkGrid,shaderChunkVerts,index);
	//Engine.setVertexDetails(shaderChunkGrid, 1, noIndices, noVerts);
	//Engine.storeIndexedModel(shaderChunkGrid,shaderChunkVerts,noVerts, index);

	shaderChunkGrid->storeVertexes(shaderChunkVerts, sizeof(vec3), noVerts);
	shaderChunkGrid->storeIndex(index, noIndices);
	shaderChunkGrid->storeLayout(3, 0, 0, 0);

	delete[] shaderChunkVerts;
	delete[] index;
}

bool CGameTerrain::superChunkIsEmpty(CSuperChunk & SC) {
	pRenderer->setShader(chunkCheckShader);
	float chunkSampleStep = SC.chunkSize / worldUnitsPerSampleUnit;
	float LoDscale = (SC.sampleStep) / (cubesPerChunkEdge);
	chunkCheckShader->setSampleCorner(SC.nwSamplePos);
	chunkCheckShader->setLoDscale(LoDscale);

	//unsigned int primitives = Engine.drawModelCount(*chunkShell);

	pRenderer->initQuery();
	chunkShell->drawNew();
	unsigned int primitives = pRenderer->query();

	//TO DO: chunkshell is coarse, create a SCshell with more points
	if ((primitives == 0)) { // ||  (primitives == shellTotalVerts * 3)) {
		return true; //outside surface
	}
	SC.tmp = true;
	return false;

}

/** Return false if no side of this potential chunk is penetratedby the isosurface.*/
bool CGameTerrain::chunkExists(vec3& sampleCorner, int LoD) {
	return true;
	//change to chunk test shader
	pRenderer->setShader(chunkCheckShader);
	float LoDscale = LoD;
	//load shader values
	chunkCheckShader->setSampleCorner(sampleCorner);
	chunkCheckShader->setLoDscale(LoDscale);

	//Draw check grid 
	//unsigned int primitives = Engine.drawModelCount(*chunkShell);

	pRenderer->initQuery();
	chunkShell->drawNew();
	unsigned int primitives = pRenderer->query();

	if (primitives == 0)
		return false;
	if (primitives == shellTotalVerts * 3)
		return false; //outside surface
	return true;
}

/*  Create a mesh for this chunk, and register it with the renderer.  */
void CGameTerrain::createChunkMesh(Chunk& chunk) {
	

	pRenderer->setShader(chunkShader);
	chunkShader->setChunkCubeSize(chunk.cubeSize);


	float LoDscale = float(1 << (chunk.LoD - 1));
	chunkShader->setChunkLoDscale(LoDscale);
	chunkShader->setChunkSamplePos(chunk.samplePos);
	//chunkShader->setSamplesPerCube(terrain->sampleScale);

	float samplesPerCube = cubeSize / worldUnitsPerSampleUnit;
	chunkShader->setSamplesPerCube(samplesPerCube);


	chunkShader->setChunkTriTable(*triTableTex);
	chunkShader->setChunkTerrainPos(chunk.terrainPos);

	int vertsPerPrimitive = 3 * chunk.noAttribs;
	int maxMCverts = 16; //The maximum vertices needed for a surface inside one MC cube.
	int nVertsOut = cubesPerChunkEdge * cubesPerChunkEdge * cubesPerChunkEdge * maxMCverts;


	CBaseBuf* terrainBuf = &multiBuf;
	CBuf* srcBuf = &((CRenderModel*)shaderChunkGrid)->buf;
	unsigned int primitives = pRenderer->getGeometryFeedback(*srcBuf, drawLinesAdjacency, (CBuf&)*tempFeedbackBuf, drawTris);





	if (primitives) {
		int outSize = primitives * 3 * sizeof(vBuf::T3DnormVert);
		totalbufsize += outSize;
		totalchunks++;

		terrainBuf->copyBuf(*tempFeedbackBuf, outSize);

		chunk.id = terrainBuf->getLastId();
		//terrainBuf->setBlockColour(chunk.id, (tmpRGBAtype&)chunk.colour);

		TDrawDetails* details = &chunk.drawDetails;
		terrainBuf->getElementData(chunk.id, details->vertStart, details->vertCount, details->childBufNo);
		details->colour = chunk.colour;

		if (chunk.LoD == 1)
			findGrassPoints(chunk);
	}
	else
		chunk.id = NULL;
	chunk.status = chSkinned;



	totalTris += (primitives * 3);
}


/** Do the necessary setup for finding grass placement points on chunks. */
void CGameTerrain::initGrassFinding() {
	//Create a buffer of evenly distributed random points for grass placement.
	grassPoints = pRenderer->createBuffer();
	std::vector <glm::vec2> points2D;
	float LoD1chunkSize = LoD1cubeSize * cubesPerChunkEdge;
	points2D = pois::generate_poisson(LoD1chunkSize, LoD1chunkSize, 7.0f /*0.25f*/, 10);
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
	pRenderer->shaderList.push_back(findPointHeightShader);
	findPointHeightShader->load(vertex, pRenderer->dataPath + "findPointHeight.vert");
	findPointHeightShader->attach();
	findPointHeightShader->setFeedbackData(1);
	findPointHeightShader->link();
	findPointHeightShader->getShaderHandles();


	/*	CBaseBuf* dummy = Engine.createBuffer();
	vec3 v(1);
	unsigned short index = 0;
	dummy->storeVertexes(&v, sizeof(vec3), 1);
	dummy->storeIndex(&index, sizeof(index), 1);
	dummy->storeLayout(3, 0, 0, 0); */

	grassMultiBuf.setSize(grassBufSize);
	//	terrain->grassMultiBuf.setInstanced(*dummy, 1);

	grassMultiBuf.setInstanced(*tree->getBuffer(), 2);
	grassMultiBuf.storeLayout(3, 3, 3, 0);


	//terrain->grassMultiBuf.storeLayout(3, 0, 0, 0);

	grassTex = pRenderer->textureManager.getTexture(pRenderer->dataPath + "grassPack.dds");

	//load the grass drawing shader
	grassShader = new CGrassShader();
	grassShader->create(pRenderer->dataPath + "grass");
	grassShader->getShaderHandles();
	grassShader->setType(userShader);
	pRenderer->shaderList.push_back(grassShader);


}

/**	Create a selection of points on the terrain surface of this chunk where grass
can be drawn. */
void CGameTerrain::findGrassPoints(Chunk & chunk) {
	float chunkSize = LoD1cubeSize * cubesPerChunkEdge;
	//load shader
	pRenderer->setShader(findPointHeightShader);
	findPointHeightShader->setCurrentY(0);
	findPointHeightShader->setSamplePosition(chunk.samplePos);
	findPointHeightShader->setSampleScale(1.0f / worldUnitsPerSampleUnit);
	findPointHeightShader->setChunkLocaliser(glm::vec3(0));

	//copy grasspoints
	CBaseBuf* pointBuf = pRenderer->createBuffer();
	pointBuf->setSize(noGrassPoints * sizeof(glm::vec3));
	pointBuf->copyBuf(*grassPoints, noGrassPoints * sizeof(glm::vec3));


	CBaseBuf* outBuf = pRenderer->createBuffer();
	outBuf->setSize(noGrassPoints * sizeof(glm::vec3));
	outBuf->setNoVerts(noGrassPoints);
	outBuf->storeLayout(3, 0, 0, 0);

	//draw chunk using transform feedback
	int points = pRenderer->getGeometryFeedback((CBuf&)*pointBuf, drawPoints, (CBuf&)*outBuf, drawPoints);

	//loop
	CBaseBuf* srcBuf = outBuf; CBaseBuf* destBuf = pointBuf; CBaseBuf* swapBuf;
	float stepHeight = chunkSize / 16;
	for (int step = 1; step <= 16; step++) {
		if (step == 16)
			findPointHeightShader->setChunkLocaliser(chunk.terrainPos);
		findPointHeightShader->setCurrentY(step * stepHeight);
		int noPrimitives = pRenderer->getGeometryFeedback((CBuf&)*srcBuf, drawPoints, (CBuf&)*destBuf, drawPoints);
		//feed points back into shader
		swapBuf = srcBuf;
		srcBuf = destBuf;
		destBuf = swapBuf;
	}

	grassMultiBuf.copyBuf(*srcBuf, srcBuf->getBufSize());
	chunk.grassId = grassMultiBuf.getLastId();

	TDrawDetails* details = &chunk.grassDrawDetails;
	grassMultiBuf.getElementData(chunk.grassId, details->vertStart, details->vertCount, details->childBufNo);

}



CGameTerrain::~CGameTerrain() {
	delete chunkCheckShader;
	delete chunkShell;

}
