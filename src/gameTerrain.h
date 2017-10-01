#pragma once

//#include "renderer\renderer.h"
#include "terrain.h"
#include "chunkCheckShader.h"
#include "chunkShader.h"
#include "findPointHeightShader.h"
#include "grassShader.h"
#include "terrainPointShader.h"

/** A subclass of CTerrain for creating this game's terrain. */

class CGameTerrain : public CTerrain {
public:
	CGameTerrain();
	void createTerrain(glm::vec2 & centre);
	~CGameTerrain();
	bool superChunkIsEmpty(CSuperChunk& SC);
	bool chunkExists(glm::vec3& sampleCorner, int LoD);
	void createChunkMesh(Chunk& chunk);
	void initChunkShell();
	void initChunkGrid(int cubesPerChunkEdge);
	void initGrassFinding();
	void initTreeFinding();
	CBaseBuf * createHeightPoints(float proximity);
	void findTreePoints(Chunk & chunk);

	void findGrassPoints(Chunk & chunk);

	CBaseBuf * createSurfacePoints(CBaseBuf * xzPoints, Chunk& chunk);
	CBaseBuf * cullPoints(CBaseBuf * points, Chunk& chunk, int mode);


	void initHeightFinder();

	void loadShaders();

	float findTerrainHeight(glm::vec3 & basePos);

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
	CBaseBuf* treePoints; ///<A 2D buffer for potential tree placement.
	unsigned int noTreePoints;


	CFindPointHeightShader* findPointHeightShader;
	CGrassShader* grassShader;
	CBaseTexture* grassTex;

	CModel* tree;

	CTerrainPointShader* terrainPointShader;
	CBuf heightFinderBuf;


	CShader* cullPointsShader;
	unsigned int hSampleOffset;
	unsigned int hSampleScale;
	unsigned int hMode;

};

const float cubeSize = 2.5f;// 2.5;// 1;//  2.5f; ///<Size of cubes in worldspace.

const unsigned int treeBufSize = 90000000; //TO DO: find proper sizes for these
const unsigned int grassBufSize = 90000000;

const float findHeightVerts = 100; ///<Number of verts findTerrainHeight checks at a time. 

CBaseBuf * createSurfacePoints(CBaseBuf * xzPoints, glm::vec3 & terrainPos);
