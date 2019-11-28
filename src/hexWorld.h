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
	void onMouseMove(int x, int y, int key);
	void onKeyDown(int key, long mod);
	void onMouseButton(int button, int action, int mods);
	void draw();
	void setAspectRatio(glm::vec2 ratio);
	CHexObject* getCursorObj();
	void update(float dt);

private:
	void tmpCreateArray();
	CHexObject* getEntity();
	void setHexCursor(CHex& pos);
	void updateCursorPath();
	void movePlayerDownPath();
	THexList* getPath() {
		return &path;
	};


	CHexArray hexArray;

	IhexWorldCallback* pCallbackApp; ///<Pointer to app used for callbacks.
	CHexRenderer hexRenderer;


	CHexObject playerModel;
	CHexObject hexCursor;
	

	bool leftMouseDown;

	bool resolving; ///<Player can't act while true.
	THexList path;
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

#define GLFW_MOUSE_BUTTON_1         0
#define GLFW_MOUSE_BUTTON_2         1
#define GLFW_MOUSE_BUTTON_3         2
#define GLFW_MOUSE_BUTTON_4         3
#define GLFW_MOUSE_BUTTON_5         4
#define GLFW_MOUSE_BUTTON_6         5
#define GLFW_MOUSE_BUTTON_7         6
#define GLFW_MOUSE_BUTTON_8         7
#define GLFW_MOUSE_BUTTON_LAST      GLFW_MOUSE_BUTTON_8
#define GLFW_MOUSE_BUTTON_LEFT      GLFW_MOUSE_BUTTON_1
#define GLFW_MOUSE_BUTTON_RIGHT     GLFW_MOUSE_BUTTON_2
#define GLFW_MOUSE_BUTTON_MIDDLE    GLFW_MOUSE_BUTTON_3

#define GLFW_RELEASE                0
#define GLFW_PRESS                  1
#define GLFW_REPEAT                 2