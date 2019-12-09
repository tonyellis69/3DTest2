#pragma once



#include "..\3DEngine\src\BaseApp.h"


#include "gameTerrain.h"
#include "terrain/terrain2.h"


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "playerObj.h"
#include "..\VMtest\src\vm.h"
#include "UI\GUIimage.h"

#include "UI\GUIrichText.h"
//#include "UI\GUIpopMenu.h"
///////////////#include "UI\GUIrichTextPanel.h"

#include "poisson.h"

#include "worldUI.h"

#include "texGen\compositor.h"
#include "texGen\texGenUI.h"

#include "utils/log.h"

#include "physics/terrainPhysObj2.h"

#include "hexRenderer.h"
#include "hexWorld.h" //replaces the above

extern CSuperChunk* dbgSC;

//extern float xAdj;
//extern float yAdj;

enum TAppMode {terrainMode, textMode, texGenMode, hexMode};

class C3DtestApp : public  CBaseApp , public ITerrainCallback,
	public IhexWorldCallback {
public:
	C3DtestApp();
	void OnMouseWheelMsg(float xoffset, float yoffset);
	
	~C3DtestApp();
	void onStart();

	void keyCheck();
	void mouseMove(int x, int y, int key);
	void onKeyDown( int key, long mod);
	void onMouseButton(int button, int action, int mods);
	//void onResize(int width, int height);
	void draw();
	void terrain2TestDraw();
	void advance(Tdirection direction);
	void Update();
	
	void initWireSCs();
	void onTerrainAdvance(Tdirection direction);

	void initHeightmapGUI();
	void updateHeightmapImage();

	void vmMessage(TvmAppMsg msg);
	void vmUpdate();
	//void showChoice();
	void HandleUImsg(CGUIbase& Control, CMessage& Message);
	bool scIntersectionCheckCallback(glm::vec3& pos, float scSize);
	bool chunkCheckCallback(glm::vec3& pos, float scSize);
	unsigned int getChunkTrisCallback(int chunkId, TChunkVert* buf);

	void createChunkMesh(Chunk2& chunk);

	void deleteChunkMesh(Chunk2& chunk);

	void drawVisibleChunks();

	void createRegion();

	void onTerrainScroll(glm::vec3& movement);

	void onResize(int width, int height);

	bool hexKeyNowCallback(int key) { return keyNow(key); };
	bool hexMouseButtonNowCallback(int button) { return mouseButtonNow(button); };

	void initHexWorld();


	string dataPath; ///<Filepath to the Data folder
	
	CModel2 cube; 
	CModel2 cube2;
	CModel2 cylinder;
	
	glm::vec2 lastMousePos; ///<Used to track current mouse movement



	CGameTerrain terrain;
	CTerrain2 terrain2;
	
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
	CBuf tempFeedbackBuf2;

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

//	CGUIrichText* textWindow;
	bool shownChoice; ///<True if we've displayed the user's choices in the console.

//	unsigned int textWindowID;
	const int optionHotText = 0x10;



	CWorldUI worldUI;



	CFont popFont;
	CFont mainFont;
	CFont mainFontBold;
	CFont popHeadFont;

	Compositor texCompositor;
	CTexGenUI texGenUI;

	TAppMode appMode;
	CBuf wireCube;
	CShader* wire2Shader;
	unsigned int hWireMVP;
	unsigned int hWireColour;
	//std::vector<glm::vec3> wireCubeVerts;

	glm::vec3 oldPos;//<Temporary clunky way to track player movement for terrain2. Tidy!

	CModel2 tmpModel2;
	ComposeTest testCompositor;
	CRenderTexture tmpBuildingTexture;

	glm::vec2 playerStartOffset;///<The adjustment required to centre terrain on the player spawn point.
	CModel2 building;


	CMultiBuf multiBuf; 
	//TO DO: temp! Think where this should go, app or CTerrain

	CHexRenderer hexRenderer;
	CHexWorld hexWorld; //TO DO: replaces the above
	
};

const float yawAng = 0.22f;
const int cubesPerChunkEdge = 16;// 16; //8 ///<Number of cubes along a chunk edge
const int chunksPerSuperChunkEdge = 4;// 4;//8;

const int terrainNoAttribs = 3;

const glm::vec4 black(0, 0, 0, 1);
const glm::vec4 white(1, 1, 1, 1);
const glm::vec4 blue(0.2, 0.2, 1, 1);
const glm::vec4 hot(0.28, 0.28, 0.47, 1);
const glm::vec4 hotSelect(1, 0.547, 0.0, 1);


//const unsigned int grassBufSize = 90000000;
//TO DO: this is waaaaaaaaaaaaaay too big, set to this value
//when I was drawing masses of grass.
