#pragma once

//#include "renderer\renderer.h"
#include "terrain.h"
#include "chunkCheckShader.h"
#include "chunkShader.h"
#include "findPointHeightShader.h"
#include "grassShader.h"

/** A subclass of CTerrain for creating this game's terrain. */

class CGameTerrain : public CTerrain {
public:
	CGameTerrain();
	~CGameTerrain();
	bool superChunkIsEmpty(CSuperChunk& SC);
	bool chunkExists(glm::vec3& sampleCorner, int LoD);
	void createChunkMesh(Chunk& chunk);
	void initChunkShell();
	void initChunkGrid(int cubesPerChunkEdge);
	void initGrassFinding();
	void findGrassPoints(Chunk & chunk);

//private:
	ChunkCheckShader* chunkCheckShader;
	ChunkShader* chunkShader;
	CModel* shaderChunkGrid; ///<3D grid of verts modelling the cubes of a chunk, used in terrain creation.
	CBaseBuf* tempFeedbackBuf;
	CBaseTexture* triTableTex; ///<Data texture for MC triangles table


	CModel* chunkShell; ///<A model to represent the outer layer of a potentianl chunk.
	int shellTotalVerts; ///<Total vertices that make up the shell.

	CBaseBuf* grassPoints; ///<A 2D buffer for potential grass placement.
	unsigned int noGrassPoints;
	CFindPointHeightShader* findPointHeightShader;
	CGrassShader* grassShader;
	CBaseTexture* grassTex;

	CModel* tree;
};

const float cubeSize = 2.5;// 1;//  2.5f; ///<Size of cubes in worldspace.

const unsigned int grassBufSize = 90000000;