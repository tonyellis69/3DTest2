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

struct THexTile;
class CHexRender {
public:
	void init();
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
	void drawUpperLineList();
	void drawSolidList();
	void drawMaskList();
	void drawExplosionList();
	void startScreenBuffer();
	void startSceneBuffer();
	void blur();
	void drawScreenBuffer();
	void drawSceneLayers();
	void setScreenSize(glm::vec2& ratio);


	CCamera* pCamera;///<Old renderer's camera - replace with own!

	CShader* lineShader;
	unsigned int hMVP;
	unsigned int hColour;
	unsigned int hWinSize;
	unsigned int hPalette;
	unsigned int hChannel;
	unsigned int hThickness;

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



	std::unordered_map<std::string, std::vector<glm::vec4>> palettes;
	
	std::vector<TLineDraw> lineDrawList;
	std::vector<TLineDraw> upperLineList;
	std::vector<TSolidDraw> solidDrawList;
	std::vector<TSplodeDraw> explosionDrawList;
	std::vector<TSolidDraw> maskList;
	std::vector<TSolidDraw> lowerMaskList;

	std::vector<std::shared_ptr<CGraphic>> graphics;

	int tmpX = 0;

	float sceneryLine = 6;
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


	CShader* blurShader;
	unsigned int hSrcTexture;
	unsigned int hHorizontal;

	CRenderTexture blurTexture[2];
	unsigned int hBlurFrameBuffer[2];


	CShader* screenBufShader;
	unsigned int hScreenBuf;
	unsigned int hScreenMask;
	unsigned int hBlurTex;
	unsigned int hX;

	CShader* sceneLayerShader;
	unsigned int hMap;
	unsigned int hModels;
	unsigned int hBlur;


	CRenderTexture screenBuffer;
	CRenderTexture screenMask;
	unsigned int hScreenFrameBuffer;
	CBuf2 screenQuad;

	CRenderTexture levelTexture;
	CRenderTexture modelTexture;

	unsigned int hStencilBuf;

	unsigned int hDepthTex;
};

struct THexTile {
	std::string name;
	TVertData vertData;
	std::vector<glm::vec4> colours;
};