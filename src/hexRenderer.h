#pragma once

#define _USE_MATH_DEFINES //for cmath
#include <vector>


#include "glm/glm.hpp"

#include "renderer/renderer.h"
#include "renderer/buf.h"
#include "importer/simpleMesh.h"
#include "hex/hexArray.h"
#include "hex/hexObject.h"


/**	A class for drawing 3D hex-based graphics. */
class IhexRendererCallback;
class CHexRenderer {
public:
	CHexRenderer();
	void start();
	void setMap();
	void draw();
	void setCallbackApp(IhexRendererCallback* pObj);
	void dollyCamera(float delta);
	void pitchCamera(float delta);
	void moveCamera(const glm::vec3& move);
	void setCameraAspectRatio(glm::vec2 ratio);
	CHex pickHex(int screenX, int screenY);
	void setMouseHex(CHex& hex);

private:
	void tmpCreateArray();
	void tmpCreateHexagonModel();

	void fillFloorplanLineBuffer();
	void fillFloorplanSolidBuffer();
	void createSolidHexModel();
	void createLineShader();
	void drawFloorPlan();
	void drawHighlights();
	void drawEntities();

	CRenderer* pRenderer;
	CBuf floorplanLineBuf;
	CBuf floorplanSolidBuf;
	CBuf solidHexBuf;
	std::vector<glm::vec3> hexModel;

	CShader* lineShader;
	unsigned int hMVP;

	CCamera camera;
	float cameraStep; ///<Amount by which camera moves in WASD.
	float cameraPitch;

	CHexArray hexArray;

	IhexRendererCallback* pCallbackObj; ///<Pointer to obj used for callbacks.

	CHex mouseHex; ///<Hex mouse is over
};


class IhexRendererCallback {
public:
	virtual CHexObject* getEntity() { return NULL; }
	virtual CHexObject* getCursorObj() { return NULL; }
};


