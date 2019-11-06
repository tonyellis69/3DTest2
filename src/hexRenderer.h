#pragma once

#define _USE_MATH_DEFINES //for cmath
#include <vector>


#include "glm/glm.hpp"

#include "renderer/renderer.h"
#include "renderer/buf.h"
#include "importer/simpleMesh.h"
#include "hex/hexArray.h"

/**	A class for drawing 3D hex-based graphics. */
class IhexCallback;
class CHexRenderer {
public:
	CHexRenderer();
	void init();
	void setMap();
	void draw();
	void setCallbackApp(IhexCallback* pApp);
	void fillFloorplanBuffer();
	void tmpCreateArray();
	void addModels(std::vector<CMesh>& meshes);

	void onMouseWheel(float delta);
	void keyCheck();


	void tmpCreateHexModel();
	void createLineShader();

	void setAspectRatio(glm::vec2 ratio);

	CRenderer* pRenderer;

	CBuf floorplanBuf;
	std::vector<glm::vec3> hexModel;

	CShader* lineShader;
	unsigned int hMVP;

	CCamera camera;

	CHexArray hexArray;

	IhexCallback* pCallbackApp; ///<Pointer to app used for callbacks.

	float cameraStep; ///<Amount by which camera moves in WASD.
	float cameraPitch; 

	CBuf tmpModelBuf;
};


class IhexCallback {
public:
	virtual bool hexKeyNowCallback(int key) { return false; };
};

#define GLFW_KEY_LEFT_SHIFT         340
