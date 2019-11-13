#pragma once

#include "hexRenderer.h"
#include "hex/hexObject.h"

/** A class encapsulating the hex-based representation of the world. */
class IhexWorldCallback;
class CHexWorld : public IhexRendererCallback {
public:
	CHexWorld();
	void setCallbackApp(IhexWorldCallback* pApp);
	void addMesh(const std::string& name, std::vector<CMesh>& meshes);
	void start();
	void keyCheck();
	void onMouseWheel(float delta);
	void onKeyDown(int key, long mod);
	void draw();
	void setAspectRatio(glm::vec2 ratio);
	

private:
	CHexObject* getEntity();
	IhexWorldCallback* pCallbackApp; ///<Pointer to app used for callbacks.
	CHexRenderer hexRenderer;

	std::map<std::string, CBuf> modelBuffers;
	CHexObject playerModel;

};

class IhexWorldCallback {
public:
	virtual bool hexKeyNowCallback(int key) { return false; };
};

#define GLFW_KEY_KP_0               320
#define GLFW_KEY_KP_1               321
#define GLFW_KEY_KP_2               322
#define GLFW_KEY_KP_3               323
#define GLFW_KEY_KP_4               324
#define GLFW_KEY_KP_5               325
#define GLFW_KEY_KP_6               326
#define GLFW_KEY_KP_7               327
#define GLFW_KEY_KP_8               328
#define GLFW_KEY_KP_9               329

#define GLFW_KEY_LEFT_SHIFT         340