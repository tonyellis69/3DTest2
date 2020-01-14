#pragma once

#define _USE_MATH_DEFINES //for cmath
#include <vector>


#include "glm/glm.hpp"

#include "renderer/renderer.h"
#include "renderer/buf.h"
#include "importer/simpleMesh.h"
#include "hex/hexArray.h"
#include "hex/hexObject.h"
#include "hex/hex.h"


/**	A class for drawing 3D hex-based graphics. */
class IhexRendererCallback;
class CHexRenderer {
public:
	CHexRenderer();
	void start();
	void setMap(CHexArray* hexArray);
	void draw();
	void setCallbackApp(IhexRendererCallback* pObj);
	void dollyCamera(float delta);
	void pitchCamera(float delta);
	void moveCamera(glm::vec3& move);
	void setCameraAspectRatio(glm::vec2 ratio);
	CHex pickHex(int screenX, int screenY);
	CBuf* addBuffer(const std::string& name);
	CBuf* getBuffer(const std::string& name);
	void setCursorPath(CHex& playerPos, CHex& cursorPos);
	void setCursorPath(THexList& path);
	THexList& getCursorPath() { return cursorPath; }

private:
	void tmpCreateHexagonModel();

	void fillFloorplanLineBuffer();
	void fillFloorplanSolidBuffer(CBuf& buf, int drawValue, float scale);
	void createSolidHexModel();
	void createLineShader();
	void drawFloorPlan();
	void drawHighlights();
	void drawEntities();

	CRenderer* pRenderer;
	CBuf floorplanLineBuf;
	CBuf floorplanSpaceBuf;
	CBuf floorplanSolidBuf;

	CBuf solidHexBuf;
	std::vector<glm::vec3> hexModel;

	CShader* lineShader;
	unsigned int hMVP;
	unsigned int hColour;

	glm::vec4 floorplanLineColour;
	glm::vec4 floorplanSpaceColour;
	glm::vec4 floorplanSolidColour;

	CCamera camera;
	float cameraPitch;

	CHexArray* hexArray;

	IhexRendererCallback* pCallbackObj; ///<Pointer to obj used for callbacks.

	std::map<std::string, CBuf> modelBuffers;
	THexList cursorPath;
};


class IhexRendererCallback {
	public:

	virtual TEntities* getEntities() { return NULL; }
	virtual CHexObject* getCursorObj() { return NULL; }
	virtual THexList* getPlayerPath() { return NULL; }
};


