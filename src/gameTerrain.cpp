#include "gameTerrain.h"

#include "renderer\renderMaterial.h"

#include "poisson.h"

#include "utils/log.h"

#include <iostream>

using namespace glm;


CGameTerrain::CGameTerrain() {
	loadShaders();

	totalScs = 0;
	passedSCs = 0;

	tempFeedbackBuf = pRenderer->createBuffer();
	tempFeedbackBuf->setSize(1000000);
}

/** Generate a landscape centred on the given point. */
void CGameTerrain::createTerrain(glm::vec2 & centre) {
	//find the terrain height at this point
	const float defaultStart = worldUnitsPerSampleUnit / 1000;
	float height = findTerrainHeight(vec3(centre.x, -defaultStart, centre.y));
	setSampleCentre(vec3(centre.x, height, centre.y));
}


/** Prepare a hollow shell of vertices to use in checks for empty chunks. */
void CGameTerrain::initChunkShell() {
	float vertsPerEdge = cubesPerChunkEdge + 1;
	shellTotalVerts = std::pow(vertsPerEdge, 3) - std::pow(vertsPerEdge - 2, 3);
	vec3* shell = new vec3[shellTotalVerts];
	int v = 0;
	//for (int y = 0; y < vertsPerEdge; y++) {
	//	for (int x = 0; x < vertsPerEdge; x++) {
	//		shell[v++] = vec3(x, y, 0);
	//		shell[v++] = vec3(x, y, cubesPerChunkEdge);
	//	}
	//	for (int z = 1; z < cubesPerChunkEdge; z++) {
	//		shell[v++] = vec3(0, y, z);
	//		shell[v++] = vec3(cubesPerChunkEdge, y, z);
	//	}
	//}

	//for (int x = 1; x < cubesPerChunkEdge; x++) {
	//	for (int z = 1; z < cubesPerChunkEdge; z++) {
	//		shell[v++] = vec3(x, 0, z);
	//		shell[v++] = vec3(x, cubesPerChunkEdge, z);
	//	}
	//}

	//chunkShell = new CRenderModel();

	//CRenderMaterial* material = new CRenderMaterial();
	//chunkShell->setMaterial(*material);
	//chunkShell->setDrawMode(drawPoints);
	//chunkShell->storeVertexes(shell, sizeof(vec3), v);
	//chunkShell->storeLayout(3, 0, 0, 0);
	//delete[] shell;
	//chunkShell->getMaterial()->setShader(chunkCheckShader);
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



	shaderChunkGrid->storeVertexes(shaderChunkVerts, sizeof(vec3), noVerts);
	shaderChunkGrid->storeIndex(index, noIndices);
	shaderChunkGrid->storeLayout(3, 0, 0, 0);

	delete[] shaderChunkVerts;
	delete[] index;
}

bool CGameTerrain::superChunkIsEmpty(CSuperChunk & SC) {
	totalScs++;
	//return false;
	pRenderer->setShader(chunkCheckShader);
	float chunkSampleStep = SC.chunkSize / worldUnitsPerSampleUnit;
	float LoDscale = (SC.sampleStep) / (cubesPerChunkEdge);
	chunkCheckShader->setShaderValue(hNWsamplePos,SC.nwSamplePos);
	chunkCheckShader->setShaderValue(hLoDscale,LoDscale);
	//cerr << "\n" << SC.nwSamplePos.x << " " << SC.nwSamplePos.y << " "
	//	<< SC.nwSamplePos.z;
	pRenderer->attachTexture(0, tmpTerrainMap.handle);
	chunkCheckShader->setShaderValue(hTerrainTexture, 0);

	//cerr << "\nLoD " << SC.LoD << " SC sampleStep " << SC.sampleStep << " LoDscale " << LoDscale;

	//if (SC.LoD == 1)
	//	cerr << "\n" << SC.tmpIndex.x << " " << SC.tmpIndex.y << " " << SC.tmpIndex.z << " "
	//	<< SC.nwSamplePos.x << " " << SC.nwSamplePos.y << " " << SC.nwSamplePos.z;

	//unsigned int primitives = Engine.drawModelCount(*chunkShell);

	pRenderer->initQuery();
	//chunkShell->drawNew();
	unsigned int primitives = pRenderer->query();

	//TO DO: chunkshell is coarse, create a SCshell with more points
	if ((primitives == 0)  ||  (primitives == shellTotalVerts ) ) {
	//	cerr << " is empty";
		return true; //outside surface
	}
	SC.nonEmpty = true;
	passedSCs++;
	//cerr << "is full.";
	return false;

}

//TO DO: currently we don't call this at all
/** Return false if no side of this potential chunk is penetratedby the isosurface.*/
bool CGameTerrain::chunkExists(vec3& sampleCorner, int LoD) {
//	return true;
	//change to chunk test shader
	pRenderer->setShader(chunkCheckShader);
	float LoDscale = LoD;
	//load shader values
	chunkCheckShader->setShaderValue(hNWsamplePos, sampleCorner);
	chunkCheckShader->setShaderValue(hLoDscale, LoDscale);

	pRenderer->attachTexture(0, tmpTerrainMap.handle);
	chunkCheckShader->setShaderValue(hTerrainTexture, 0);

	//Draw check grid 
	//unsigned int primitives = Engine.drawModelCount(*chunkShell);

	pRenderer->initQuery();
	//chunkShell->drawNew();
	unsigned int primitives = pRenderer->query();

	if (primitives == 0)
		return false;
	//if (primitives == shellTotalVerts * 3)
	//	return false; //outside surface
	std::cerr << "\n" << primitives;
	return true;
}

/*  Create a mesh for this chunk, and register it with the renderer.  */
void CGameTerrain::createChunkMesh(Chunk& chunk) {
	

	pRenderer->setShader(chunkShader);
	chunkShader->setShaderValue(hChunkCubeSize,chunk.cubeSize);


	float LoDscale = float(1 << (chunk.LoD - 1));
	chunkShader->setShaderValue(hChunkLoDscale,LoDscale);
	chunkShader->setShaderValue(hChunkSamplePos, chunk.samplePos);
	//chunkShader->setSamplesPerCube(terrain->sampleScale);

	//sysLog << "\nLod " << chunk.LoD << " cubesize " << chunk.cubeSize;

	float samplesPerCube = cubeSize / worldUnitsPerSampleUnit;
	chunkShader->setShaderValue(hSamplesPerCube, samplesPerCube);


	chunkShader->setShaderValue(hChunkTriTable, *triTableTex);
	chunkShader->setShaderValue(hChunkTerrainPos, chunk.terrainPos);

	pRenderer->attachTexture(0, tmpTerrainMap.handle);
	chunkShader->setShaderValue(hChunkTerrainTexture, 0);

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
		
		TDrawDetails* details = &chunk.drawDetails;
		terrainBuf->getElementData(chunk.id, details->vertStart, details->vertCount, details->childBufNo);
		details->colour = chunk.colour;

		if (chunk.LoD == 1) {
			findTreePoints(chunk);
		//	findGrassPoints(chunk);
		}
	}
	else
		chunk.id = NULL;
	chunk.status = chSkinned;



	totalTris += (primitives * 3);
}


/** Do the necessary setup for finding grass placement points on chunks. */
void CGameTerrain::initGrassFinding() {
	//Create a buffer of evenly distributed random points for grass placement.
	grassPoints = createHeightPoints(0.25f);
	noGrassPoints = grassPoints->getNoVerts();

	/*	CBaseBuf* dummy = Engine.createBuffer();
	vec3 v(1);
	unsigned short index = 0;
	dummy->storeVertexes(&v, sizeof(vec3), 1);
	dummy->storeIndex(&index, sizeof(index), 1);
	dummy->storeLayout(3, 0, 0, 0); */

	grassMultiBuf.setSize(grassBufSize);
	//	terrain->grassMultiBuf.setInstanced(*dummy, 1);

	//grassMultiBuf.setInstanced(*tree->getBuffer(), 2);
	//grassMultiBuf.storeLayout(3, 3, 3, 0);


	grassMultiBuf.storeLayout(3, 0, 0, 0);
}


/** Do the necessary setup for finding tree placement points on chunks. */
void CGameTerrain::initTreeFinding() {
	//Create a buffer of evenly distributed random points for tree placement.
	treePoints = createHeightPoints(2.0f);
	noTreePoints = treePoints->getNoVerts();

	treeMultiBuf.setSize(treeBufSize);

	treeMultiBuf.setInstanced(*tree->getBuffer(), 2);
	treeMultiBuf.storeLayout(3, 3, 3, 0);
}



/** Create a buffer full of random points evenly distributed over the area of a LoD1 chunk. */
CBaseBuf* CGameTerrain::createHeightPoints(float proximity) {
	CBaseBuf* points = pRenderer->createBuffer();
	std::vector <glm::vec2> points2D;
	float LoD1chunkSize = LoD1cubeSize * cubesPerChunkEdge;
	points2D = pois::generate_poisson(LoD1chunkSize, LoD1chunkSize, proximity, 10);
	int noPoints = points2D.size();
	std::vector < glm::vec3> points3D(noPoints);
	for (int p = 0; p < noPoints; p++) {
		points3D[p].x = points2D[p].x;
		points3D[p].z = points2D[p].y;
		points3D[p].y = -FLT_MAX;
	}
	points->storeVertexes(points3D.data(), sizeof(glm::vec3) * noPoints, noPoints);
	points->storeLayout(3, 0, 0, 0);
	return points;
}


/**	Create a selection of points on the terrain surface of this chunk where trees can be drawn. */
void CGameTerrain::findTreePoints(Chunk & chunk) {
	CBaseBuf* mappedPoints = createChunkSurfacePoints(treePoints, chunk);

	mappedPoints = cullPoints(mappedPoints, chunk, 1);

	treeMultiBuf.copyBuf(*mappedPoints,mappedPoints->getBufSize());
	chunk.treeId = treeMultiBuf.getLastId();

	TDrawDetails* details = &chunk.treeDrawDetails;
	treeMultiBuf.getElementData(chunk.treeId, details->vertStart, details->vertCount, details->childBufNo);
} 

/**	Create a selection of points on the terrain surface of this chunk where grass can be drawn. */
void CGameTerrain::findGrassPoints(Chunk & chunk) {
	CBaseBuf* mappedPoints = createChunkSurfacePoints(grassPoints, chunk);

	mappedPoints = cullPoints(mappedPoints, chunk, 2);

	grassMultiBuf.copyBuf(*mappedPoints, mappedPoints->getBufSize());
	chunk.grassId = grassMultiBuf.getLastId();

	TDrawDetails* details = &chunk.grassDrawDetails;
	grassMultiBuf.getElementData(chunk.grassId, details->vertStart, details->vertCount, details->childBufNo);
}




/** Return a buffer of the given xz points, elevated to the surface of the given chunk. */
CBaseBuf* CGameTerrain::createChunkSurfacePoints(CBaseBuf* xzPoints, Chunk& chunk) {
	float chunkSize = LoD1cubeSize * cubesPerChunkEdge;

	pRenderer->setShader(findPointHeightShader);
	findPointHeightShader->setShaderValue(hCurrentY,0);
	findPointHeightShader->setShaderValue(hChunkSamplePosition,chunk.samplePos);
	findPointHeightShader->setShaderValue(hFPHSampleScale, 1.0f / worldUnitsPerSampleUnit);
	findPointHeightShader->setShaderValue(hChunkLocaliser, glm::vec3(0));

	pRenderer->attachTexture(0, tmpTerrainMap.handle);
	findPointHeightShader->setShaderValue(hFPHterrainTexture, 0);

	//copy points
	int noPoints = xzPoints->getNoVerts();
	CBaseBuf* pointBuf = pRenderer->createBuffer();
	pointBuf->setSize(noPoints * sizeof(glm::vec3));
	pointBuf->copyBuf(*xzPoints, noPoints * sizeof(glm::vec3));


	CBaseBuf* outBuf = pRenderer->createBuffer();
	outBuf->setSize(noPoints * sizeof(glm::vec3));
	outBuf->setNoVerts(noPoints);
	outBuf->storeLayout(3, 0, 0, 0);

	//draw chunk using transform feedback
	int points = pRenderer->getGeometryFeedback((CBuf&)*pointBuf, drawPoints, (CBuf&)*outBuf, drawPoints);

	//loop
	CBaseBuf* srcBuf = outBuf; CBaseBuf* destBuf = pointBuf; CBaseBuf* swapBuf;
	float stepHeight = chunkSize / 16;
	for (int step = 1; step <= 16; step++) {
		if (step == 16)
			findPointHeightShader->setShaderValue(hChunkLocaliser, chunk.terrainPos);
		findPointHeightShader->setShaderValue(hCurrentY, step * stepHeight);
		int noPrimitives = pRenderer->getGeometryFeedback((CBuf&)*srcBuf, drawPoints, (CBuf&)*destBuf, drawPoints);
		//feed points back into shader
		swapBuf = srcBuf;
		srcBuf = destBuf;
		destBuf = swapBuf;
	}
	//TO DO: look into freeing up some of these buffers!

	return srcBuf;
}

/** Remove some of the points in the given point field, to create a more organic pattern. */
CBaseBuf * CGameTerrain::cullPoints(CBaseBuf * points, Chunk & chunk, int mode) {

	int noPoints = points->getNoVerts();
	CBaseBuf* outBuf = pRenderer->createBuffer();
	outBuf->setSize(noPoints * sizeof(glm::vec3));
	outBuf->setNoVerts(noPoints);
	outBuf->storeLayout(3, 0, 0, 0);

	float sampleScale = 1 / worldUnitsPerSampleUnit;

	pRenderer->setShader(cullPointsShader);
	cullPointsShader->setShaderValue(hSampleOffset, sampleOffset);
	cullPointsShader->setShaderValue(hSampleScale, sampleScale);
	cullPointsShader->setShaderValue(hMode, mode);

	//draw chunk using transform feedback
	int nPoints = pRenderer->getGeometryFeedback((CBuf&)*points, drawLines, (CBuf&)*outBuf, drawPoints);
	//NB can't reset actual size of the buffer as this will erase data
	outBuf->reduceReportedSize(nPoints * sizeof(glm::vec3));
	outBuf->setNoVerts(nPoints);

	return outBuf;
}

/** Initialise shader and buffer required to run a terrain height-finding query. */
void CGameTerrain::initHeightFinder() {


	//create verts
	vec3* v = new vec3[findHeightVerts];
	for (int x = 0; x < findHeightVerts; x++)
		v[x] = vec3(0, x, 0);

	heightFinderBuf.storeVertexes(v, sizeof(vec3) * findHeightVerts, findHeightVerts);
	heightFinderBuf.storeLayout(3, 0, 0, 0);
	delete v;
}

void CGameTerrain::loadShaders() {
	//load chunk shader
	char* chunkFeedbackStrs[2];
	chunkFeedbackStrs[0] = "gl_Position";
	chunkFeedbackStrs[1] = "normal";
	chunkShader = pRenderer->createShader(pRenderer->dataPath + "chunk", chunkFeedbackStrs, 2);
	chunkShader->setType(userShader);

	//Upload data texture for chunk shader
	triTableTex = pRenderer->createDataTexture(intTex, 16, 256, &triTable);
	pRenderer->setShader(chunkShader);
	hChunkCubeSize = chunkShader->getUniformHandle("cubeSize");
	hChunkLoDscale = chunkShader->getUniformHandle("LoDscale");
	hChunkSamplePos = chunkShader->getUniformHandle("samplePos");
	hChunkTriTable = chunkShader->getUniformHandle("triTableTex");
	hChunkTerrainPos = chunkShader->getUniformHandle("terrainPos");
	hSamplesPerCube = chunkShader->getUniformHandle("samplesPerCube");
	hChunkTerrainTexture = chunkShader->getUniformHandle("terrainTexture");
	chunkShader->setShaderValue(hChunkTriTable, *triTableTex);

	//load chunkCheck shader
	chunkCheckShader = pRenderer->createShader(pRenderer->dataPath + "chunkCheck");
	pRenderer->setShader(chunkCheckShader);
	hNWsamplePos = chunkCheckShader->getUniformHandle("nwSamplePos");
	hLoDscale = chunkCheckShader->getUniformHandle("LoDscale");
	hTerrainTexture = chunkCheckShader->getUniformHandle("terrainTexture");

	//load te point finding shader
	char* fPointFeedbackStrs[1];
	fPointFeedbackStrs[0] = "newPoint";
	findPointHeightShader = pRenderer->createShader(pRenderer->dataPath + "findPointHeight", fPointFeedbackStrs, 1);
	hCurrentY = findPointHeightShader->getUniformHandle("currentY");
	hChunkSamplePosition = findPointHeightShader->getUniformHandle("chunkSamplePosition");
	hFPHSampleScale = findPointHeightShader->getUniformHandle("sampleScale");
	hChunkLocaliser = findPointHeightShader->getUniformHandle("chunkLocaliser");
	hFPHterrainTexture = findPointHeightShader->getUniformHandle("terrainTexture");

	//TO DO: temporarily commented out while I try to remove dependency on soil.h
	//grassTex = pRenderer->textureManager.getTexture(pRenderer->dataPath + "grassPack.dds");

	//load the grass drawing shader
	grassShader = pRenderer->createShader(pRenderer->dataPath + "grass");
	hGrassMVP = grassShader->getUniformHandle("mvpMatrix");
	hGrassTexure = grassShader->getUniformHandle("grassTex");
	hGrassTime = grassShader->getUniformHandle("time");


	//load terrain surface point shader


	char* pointFeedbackStrs[1];
	pointFeedbackStrs[0] = "result";
	terrainPointShader = pRenderer->createShader(pRenderer->dataPath + "terrainPoint", pointFeedbackStrs, 1);
	terrainPointShader->setType(userShader);
	pRenderer->setShader(terrainPointShader);
	hPointSampleBase = terrainPointShader->getUniformHandle("sampleBase");
	hPointOffsetScale = terrainPointShader->getUniformHandle("offsetScale");
	hPSTerrainTexture = terrainPointShader->getUniformHandle("terrainTexture");

	//load heightpoint culling shader
	char* feedbackStrs[1];
	feedbackStrs[0] = "gl_Position";
	cullPointsShader = pRenderer->createShader(pRenderer->dataPath + "cullPoints", feedbackStrs,1);
	cullPointsShader->setType(userShader);
	pRenderer->setShader(cullPointsShader);
	hSampleOffset = cullPointsShader->getUniformHandle("sampleOffset");
	hSampleScale = cullPointsShader->getUniformHandle("sampleScale");
	hMode = cullPointsShader->getUniformHandle("mode");

	wireBoxShader = pRenderer->createShader(pRenderer->dataPath + "wireBox");
	wireBoxShader->setType(userShader);
	hBoxColour = wireBoxShader->getUniformHandle("colour");
	hBoxMVP = wireBoxShader->getUniformHandle("mvpMatrix");
}

float CGameTerrain::findTerrainHeight(glm::vec3& basePos) {
	pRenderer->setShader(terrainPointShader);
	float offsetScale = 1 / worldUnitsPerSampleUnit;
	terrainPointShader->setShaderValue(hPointOffsetScale,offsetScale);

	pRenderer->attachTexture(0, tmpTerrainMap.handle);
	terrainPointShader->setShaderValue(hPSTerrainTexture, 0);

	vec3 startPos = basePos;
	CBaseBuf* heightResultsBuf = pRenderer->createBuffer();
	heightResultsBuf->setSize(sizeof(float) * findHeightVerts);

	float* heightResults = new float[findHeightVerts];
	float terrainHeight = 0;; const float MCvertexTest = 0.5f;

	for (int step = 0; step < 100; step++) {
		terrainPointShader->setShaderValue(hPointSampleBase, startPos);
		pRenderer->getGeometryFeedback(heightFinderBuf, drawPoints,(CBuf&) *heightResultsBuf, drawPoints);

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

CGameTerrain::~CGameTerrain() {
//	delete chunkCheckShader;
//	delete chunkShell;

}
