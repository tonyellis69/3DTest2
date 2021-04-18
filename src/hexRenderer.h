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
#include "hex/hexObject.h"
#include "hex/hex.h"


#include "lineModel.h"




/**	A class for drawing 3D hex-based graphics. */
class CHexRenderer {
public:
	CHexRenderer();
	void init();
	void setMap(CHexArray* hexArray);
	void drawFloorPlan();
	void drawPath(THexList* path, glm::vec4& pathStartColour, glm::vec4& pathEndColour);
	void dollyCamera(float delta);
	void pitchCamera(float delta);
	void moveCamera(glm::vec3& move);
	void setCameraAspectRatio(glm::vec2 ratio);
	std::tuple <CHex, glm::vec3> pickHex(int screenX, int screenY);
	void loadMesh(const std::string& name, const std::string& fileName);
	CLineModel getLineModel(const std::string& name);
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

	void drawExplosion( glm::vec3& pos, float& lifeTime, float& size, float& timeOut);

	void drawLineModel(CLineModel& lineModel);

	void pointCamera(glm::vec3& dir);
	void setCameraHeight(float z);
	void setCameraPos(glm::vec3& pos);
	void setCameraPitch(float pitch);

	CShader* hexLineShader;
	CShader* hexSolidShader;
	CShader* lineShader;
	CShader* visibilityShader;
	CShader* explosionShader;


	
private:
	void tmpCreateHexagonModel();

	void fillFloorplanLineBuffer();
	void fillFloorplanSolidBuffer(CBuf2& buf, int drawValue, float scale);
	void createSolidHexModel();
	void createLineShader();
	void createHexShader();
	void createVisibilityShader();

	void createExplosionShader();


	void drawNode2(TModelNode& node, glm::mat4& parentMatrix, CBuf2* buf);
	
	void createFogBuffer(int w, int h);

	unsigned int hFogBuffer;
	unsigned int hFogTex;

	CRenderer* pRenderer;
	CBuf2 floorplanLineBuf;
	CBuf2 floorplanSpaceBuf;
	CBuf2 floorplanSolidBuf;

	CBuf2 solidHexBuf;
	std::vector<glm::vec3> hexModel;

	CLineModel* solidHex;

	
	unsigned int hMVP;
	unsigned int hColour;
	unsigned int hWinSize;

	CShader* lineShaderBasic;
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

	glm::vec4 floorplanLineColour;
	glm::vec4 floorplanSpaceColour;
	glm::vec4 floorplanSolidColour;

	CCamera camera;
	float cameraPitch;
	bool followCam;
	glm::vec3 followCamVec; ///<Vector from target point on plane to camera
	float screenScrollSpeed;

	CHexArray* hexArray;

	std::list<CBuf2> modelBuffers;
	//std::list<CBuf> modelBuffers2;

	std::map<std::string, CLineModel> lineModels;


	THexList cursorPath;

	CImporter importer;

	CBuf2 hexShaderBuf;

	CBuf2 unitLineBuf; //Should be temp, holds verts to draw sightline

	CBuf2 explosionBuf; //Holds dummy verts for draw calls

};



extern CHexRenderer hexRendr2;
