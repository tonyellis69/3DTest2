#pragma once

//#include "renderer\renderer.h"
#include "terrain.h"
#include "chunkCheckShader.h"

/** A subclass of CTerrain for creating this game's terrain. */

class CGameTerrain : public CTerrain {
public:
	CGameTerrain();
	~CGameTerrain();
	bool superChunkIsEmpty(CSuperChunk& SC);
	bool chunkExists(glm::vec3& sampleCorner, int LoD);
	void initChunkShell();

//private:
	ChunkCheckShader* chunkCheckShader;
	CModel* chunkShell; ///<A model to represent the outer layer of a potentianl chunk.
	int shellTotalVerts; ///<Total vertices that make up the shell.
};