#include "gameTerrain.h"

#include "renderer\renderMaterial.h"

using namespace glm;

CGameTerrain::CGameTerrain() {
	//load chunkCheck shader
	chunkCheckShader = new ChunkCheckShader();
	chunkCheckShader->create(pRenderer->dataPath + "chunkCheck");
	chunkCheckShader->getShaderHandles();

	
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



CGameTerrain::~CGameTerrain() {
	delete chunkCheckShader;
	delete chunkShell;

}
