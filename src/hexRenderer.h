#pragma once

#define _USE_MATH_DEFINES //for cmath
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"

#include "renderer/renderer.h"
//#include "renderer/buf.h"
#include "buf2.h"
#include "importer/importer.h"
#include "hex/hexArray.h"
#include "entity.h"
#include "hex/hex.h"


//#include "lineModel.h"

struct TExplode {
	glm::vec3 pos;
	float lifeTime;
	float size;
	float timeOut;
	int particleCount;
	float seed;
};


/**	A class for drawing 3D hex-based graphics. */
class CHexRenderer {
public:
	CHexRenderer();
	void init();
	void setMap(CHexArray* hexArray);
	void drawFloorPlan();
	void drawPath(THexList* path, glm::vec4& pathStartColour, glm::vec4& pathEndColour);
	bool dollyCamera(float delta);
	void pitchCamera(float delta);
	void moveCamera(glm::vec3& move);
	void setCameraAspectRatio(glm::vec2 ratio);
	std::tuple <CHex, glm::vec3> pickHex(int screenX, int screenY);
	void setCursorPath(CHex& playerPos, CHex& cursorPos);
	void setCursorPath(THexList& path);
	THexList& getCursorPath() { return cursorPath; }
	void toggleFollowCam();
	bool following() {
		return followCam;
	}

	void followTarget(glm::vec3& target);
	void attemptScreenScroll(glm::i32vec2& mousePos, float dT);

	glm::vec3 castFromCamToHexPlane(glm::vec3& ray);

	glm::i32vec2 worldPosToScreen(glm::vec3& worldPos);

	void highlightHex(CHex& hex);

	void updateFogBuffer();

	void updateMapVerts();

	void drawSightLine(glm::vec3& posA, glm::vec3& posB);

	void drawExplosion(TExplode& e);

	//void drawLineModel(TModelMesh& mesh);
	//void drawSolidModel(TModelMesh& model);

	void pointCamera(glm::vec3& dir);
	void setCameraHeight(float z);
	void setCameraPos(glm::vec3& pos);
	void setCameraPitch(float pitch);

	bool isOnScreen(glm::vec3& pos);

	CCamera camera;

	CShaderOld* hexLineShader;
	CShaderOld* hexSolidShader;
	CShaderOld* lineShader;
	CShaderOld* visibilityShader;
	CShaderOld* explosionShader;
	CShaderOld* filledShader;

	const int numExplosionParticles = 200;
	
private:
	void tmpCreateHexagonModel();

	void fillFloorplanLineBuffer();
	void fillFloorplanSolidBuffer(CBuf2& buf, int drawValue, float scale);
	void createSolidHexModel();
	void createLineShader();
	void createHexShader();
	void createVisibilityShader();

	void createExplosionShader();
	void createFilledShader();

	//TO DO: scrap this, but currently it's used in highLightHex
	void drawModel(TModelData& node, glm::mat4& parentMatrix, CBuf2* buf, glm::vec4& colour);

	
	void createFogBuffer(int w, int h);

	unsigned int hFogBuffer;
	unsigned int hFogTex;

	CRenderer* pRenderer;
	CBuf2 floorplanLineBuf;
	CBuf2 floorplanSpaceBuf;
	CBuf2 floorplanSolidBuf;

	CBuf2 solidHexBuf;
	std::vector<glm::vec3> hexModel;

	CModel* solidHex;

	//glm::vec2 screenRect; ///<Screen dimensions in worldspace

	
	unsigned int hMVP;
	unsigned int hColour;
	unsigned int hWinSize;
	unsigned int hPalette;

	CShaderOld* lineShaderBasic;
	unsigned int hMVPb;
	unsigned int hColourb;


	unsigned int hHexMVP;
	unsigned int hGridSize;
	unsigned int hFogTexUniform;

	unsigned int hHexMVPs;
	unsigned int hGridSizes;
	unsigned int hFogTexUniforms;

	unsigned int hHexMVPv;
	unsigned int hGridSizev;
	unsigned int hEffectsTexUniformv;

	unsigned int hPos;
	unsigned int hExpMVP;
	unsigned int hLifeTime;
	unsigned int hSize;
	unsigned int hTimeOut;
	unsigned int hSeed;

	unsigned int hFillMVP;
	unsigned int hFillColour;

	glm::vec4 floorplanLineColour;
	glm::vec4 floorplanSpaceColour;
	glm::vec4 floorplanSolidColour;


	float cameraPitch;
	bool followCam;
	glm::vec3 followCamVec; ///<Vector from target point on plane to camera
	float screenScrollSpeed = 25.0f;

	CHexArray* hexArray;

	//std::map<std::string, CLineModel> lineModels;


	THexList cursorPath;

	CImporter importer;

	CBuf2 hexShaderBuf;

	CBuf2 unitLineBuf; //Should be temp, holds verts to draw sightline

	CBuf2 explosionBuf; //Holds dummy verts for draw calls

	std::vector<glm::vec4> testColours;
};



extern CHexRenderer hexRendr2;
