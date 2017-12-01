#pragma once

#include "BaseApp.h"

#include "gameTerrain.h"



#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "playerObj.h"
#include "..\VMtest\src\vm.h"
#include "UI\GUIimage.h"

#include "UI\GUIconsole.h"
#include "UI\GUImenu.h"

#include "poisson.h"

extern CSuperChunk* dbgSC;

extern float xAdj;
extern float yAdj;

class C3DtestApp : public  CBaseApp {
public:
	C3DtestApp();
	~C3DtestApp();
	void onStart();

	void keyCheck();
	void mouseMove(int x, int y, int key);
	void onKeyDown( int key, long mod);
	//void onResize(int width, int height);
	void draw();
	void advance(Tdirection direction);
	void Update();
	
	void initWireSCs();
	void onTerrainAdvance(Tdirection direction);

	void initHeightmapGUI();
	void updateHeightmapImage();

	void initTextWindow();

	void vmMessage(TVMmsg msg);
	void vmUpdate();
	void showChoice();
	void HandleUImsg(CGUIbase& Control, CMessage& Message);


	string dataPath; ///<Filepath to the Data folder
	
	CModel* cube; //TO DO: get rid of this
	
	glm::vec2 lastMousePos; ///<Used to track current mouse movement



	CGameTerrain terrain;
	
	
	double oldTime;

	bool mouseLook; ///<True if mouselook mode on.
	glm::vec2 oldMousePos;


	CCamera fpsCam;
	bool fpsOn; ///<True when we're using the fps came

	glm::i32vec3 selectChk; ///<Index of the chunk we're selecting

	CModel* chunkShell; ///<A model to represent the outer layer of a potentianl chunk.
	int shellTotalVerts; ///<Total vertices that make up the shell.
	CModel* shaderChunkGrid; ///<3D grid of verts modelling the cubes of a chunk, used in terrain creation.
	CModel* wireSCs; ///<A model representing the SC wireframe.

	bool supWire; ///<True when superchunk wireframe is on.

	int chunkCall;

	CBaseBuf* tempFeedbackBuf; 

	float bigGap;

	int tmpSCno;

	CSkyDome* skyDome;

	//ChunkShader* chunkShader;
	//CBaseTexture* triTableTex; ///<Data texture for MC triangles table
	

	CBasePhysObj* physCube;

	bool tmp;

	CPlayerObj playerObject;
	
	CBasePhysObj* playerPhys;

	CGUIimage* heightmapImage;
	CBaseTexture* heightmapTex;

	CShader* terrain2texShader; 
	unsigned int hTer2TexNwSampleCorner;
	unsigned int hTer2TexPixelScale;

	int SCpassed, SCrejected;

	//CTerrainPointShader* terrainPointShader;
	//CBuf heightFinderBuf;

	CBaseBuf* grassPoints; ///<A 2D buffer for potential grass placement.
	unsigned int noGrassPoints;


//	CFindPointHeightShader* findPointHeightShader;




	
	CModel* tree;

	CGUIconsole* textWindow;
	bool shownChoice; ///<True if we've displayed the user's choices in the console.
	unsigned int menuID; ///<Identifies the choice menu control.
	CGUImenu* choiceMenu;

};

const float yawAng = 0.22f;
const int cubesPerChunkEdge = 16;// 16; //8 ///<Number of cubes along a chunk edge
const int chunksPerSuperChunkEdge = 4;// 4;//8;

const int terrainNoAttribs = 3;


//const unsigned int grassBufSize = 90000000;
//TO DO: this is waaaaaaaaaaaaaay too big, set to this value
//when I was drawing masses of grass.
