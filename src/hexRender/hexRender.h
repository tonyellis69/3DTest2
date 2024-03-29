#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "hex/hexArray.h"
#include "renderer/mesh.h"
#include "model.h"
#include "renderer/camera.h"
#include "renderer/shader.h"

#include "graphics/shader2.h"

#include "drawData.h"
#include "graphics.h"

#include "../gameEvent.h"

struct THexTile;
class CHexRender {
public:
	void onSpawn();
	void recompileShader();
	void onEvent(CGameEvent& e);
	void loadMap(CHexArray* hexArray);
	void addHexTile(const std::string& name, TVertData& vertData, std::vector<glm::vec4>& colours);
	void drawMap();
	void drawMeshLine(TMeshRec& mesh);
	void drawMeshSolid(TMeshRec& mesh);
	void drawExplosion(int particleCount);
	void storePalette(const std::string& name, std::vector<glm::vec4> palette);
	std::vector<glm::vec4>* getPalette(const std::string& name);
	void resetDrawLists();
	void drawLineList();
	void drawLineListDBG();
	void drawUpperLineList();
	void makeGlowShapes();
	void drawSolidList();
	void drawMaskList();
	void drawExplosionList();
	void startScreenBuffer();
	void startSceneBuffer();
	void blur();
	void drawScreenBuffer();
	void drawSceneLayers();
	void drawGlow();
	void setScreenSize(glm::vec2& ratio);
	glm::i32vec2 getScreenSize() {
		return screenSize;
	}

	void resizeBlurTextures();

	void setCameraPos(float x, float y);

	void drawModelAt(CModel& model, glm::vec3& pos);

	void loadLineList(TLineDraw& listEntry);
	void loadLineListDbg(TLineDraw& listEntry);
	void loadUpperLineList(TLineDraw& listEntry);
	void loadSolidList(TSolidDraw& listEntry);
	void loadUpperMaskList(TSolidDraw& listEntry);
	void loadLowerMaskList(TSolidDraw& listEntry);
	void loadExplosionDrawList(TSplodeDraw& listEntry);

	bool dollyCamera(float delta);
	void pitchCamera(float delta);
	void moveCamera(glm::vec3& move);
	void pointCamera(glm::vec3& dir);
	void setCameraHeight(float z);
	void setCameraPos(glm::vec3& pos);
	void setCameraPitch(float pitch);
	void setCameraAspectRatio(glm::vec2& ratio, float fov);
	glm::vec3 screenToWS(int screenX, int screenY);
	glm::vec3 castFromCamToHexPlane(glm::vec3& ray);


	//CCamera* pCamera;///<Old renderer's camera - replace with own!
	CCamera camera;
	float cameraPitch;


	CShader* lineShader;
	unsigned int hMVP;
	unsigned int hColour;
	unsigned int hWinSize;
	unsigned int hPalette;
	unsigned int hChannel;
	unsigned int hThickness;
	unsigned int hSmoothing;
	unsigned int hScale;
	unsigned int hSolid;

	CShader* filledShader;
	unsigned int hMVPF;
	unsigned int hPaletteF;

	CShader* maskShader;
	unsigned int hMaskMVP;


	CShader* splodeShader;
	unsigned int hPos;
	unsigned int hExpMVP;
	unsigned int hLifeTime;
	unsigned int hSize;
	unsigned int hTimeOut;
	unsigned int hSeed;
	unsigned int hExpPalette;

	CShader* blurShader;
	unsigned int hSrcTexture;
	unsigned int hHorizontal;
	unsigned int hKernelSize;
	unsigned int hSigma;



	std::unordered_map<std::string, std::vector<glm::vec4>> palettes;
	


	std::vector<std::shared_ptr<CGraphic>> graphics;

	int tmpX = 0;

	float tmpLineThickness = 1;// 2; //20;// 6; // 2;
	float tmpLineSmooth = 0;// 0.1f;// 1.5f;// 0; // 1.5f;
	float tmpLineSolid = 0.1f;// 0.9f;
	int tmpKernel = 9;
	float tmpSigma = 4.0;
	int tmpBlurs = 4;
	int tmpBlurTextDivisor = 4;
	float tmpFade = 0.2f;

	float sceneryLine = 4;
	float modelLine = 2; // 1.5f;


private:
	void addToMapBuf(std::vector<vc>& v, std::vector<unsigned int>& i, CHex& hex, int tile);
	unsigned int getColourNo(glm::vec4& colour);

	std::vector<THexTile> hexTiles;
	CHexArray* pHexArray = nullptr;


	CBuf2 mapBuf;
	std::vector<glm::vec4> uniqueTileColours;

	std::unordered_map<std::string, std::shared_ptr<CShader>> shaders;

	const int numExplosionParticles = 200;
	CBuf2 explosionBuf; //Holds dummy verts for draw calls

	const int maxDrawListSize = 30;
	std::vector<TLineDraw> lineDrawList;
	std::vector<TLineDraw> lineDrawListDBG;
	std::vector<TLineDraw> upperLineList;
	std::vector<TSolidDraw> solidDrawList;
	std::vector<TSplodeDraw> explosionDrawList;
	std::vector<TSolidDraw> maskList;
	std::vector<TSolidDraw> lowerMaskList;
	int lineDrawSize;
	int upperLineSize;
	int solidDrawSize;
	int explosionDrawSize;
	int maskSize;
	int lowerMaskSize;
	//NB: can prob scrap sizes, emplace_back tracks this for you


	CRenderTexture blurTexture[2];
	unsigned int hBlurFrameBuffer[2];


	CShader* screenBufShader;
	unsigned int hScreenBuf;
	unsigned int hScreenMask;
	unsigned int hBlurTex;
	unsigned int hX;

	CShader* sceneLayerShader;
	unsigned int hBlur;
	unsigned int hFade;


	CRenderTexture screenBuffer;
	CRenderTexture screenMask;
	unsigned int hScreenFrameBuffer;
	CBuf2 screenQuad;

	CRenderTexture levelTexture;
	CRenderTexture modelTexture;

	unsigned int hStencilBuf;

	unsigned int hDepthTex;

	glm::i32vec2 screenSize;
};

struct THexTile {
	std::string name;
	TVertData vertData;
	std::vector<glm::vec4> colours;
};