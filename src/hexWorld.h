#pragma once

#include <random>


#include "level\level.h"

//#include "mapMaker.h"

//#include "hexRenderer.h"

#include "robot.h"
#include "playerHexObj.h"

#include "UI/GUIlabel2.h"


//#include "gameEvents.h"
#include "messaging/messenger.h"

#include "gameTextWin.h"

#include "gameState.h"

#include "physics/hexPhysics.h"

#include "renderer/drawText.h" // temp for testing

#include "mapEdit/mapEdit.h"

#include "listen/listen.h"

#include "hexRender/hexRender.h"

#include "gameEvent.h"
#include "physEvent.h"

#include "level\levelGen.h"

#include "modules/gameMode.h"

enum TViewMode {gameView, devView, keepView};
enum TMsgType {msgId,msgId2,msgId3};

enum TCameraMode {camNone, camFollow, camFree, camFixed};

/** A class encapsulating the hex-based representation of the world. */
class CHexWorld { 
public:
	CHexWorld();
	void onEvent(CGUIevent& e);
	void onEvent(CGameEvent& e);
	void onEvent(CPhysicsEvent& e);
	void addMesh(const std::string& name, const std::string& fileName);
	void addHexTile(const std::string& name, const std::string& fileName, std::vector<glm::vec4>& colours);
	void makeMap();
	void deleteMap();
	void startGame();
	void startProcTest();
	void moveCamera(glm::vec3& direction);

	void calcMouseWorldPos();
	void draw();
	void setAspectRatio(glm::vec2& ratio);
	void update(float dt);
	void toggleView();
	void toggleEditMode();
	void onUndo();
	void onRedo();


	void prepMapEntities();



	CGUIlabel2* hexPosLbl;

	std::string lblText;

	CDrawText drawTxtTest; //temp!

	bool editMode = false;
	bool procTestMode = false;

	//FIXME: stuff moved to make accessible to working module
	CHexRender hexRender; ///<New hex renderer.
	CEntity* hexCursor = NULL;;
	bool zoom2fit = false;
	CHexPhysics physics;
	CPlayerObject* playerObj = NULL;
	CEntity* pBotZero = NULL;
	CModel reticule;
	float zoomScale = 1.0f;
	bool mapDragging = false;
	glm::i32vec2 mousePos;
	glm::i32vec2 lastMousePos;
	glm::vec3 mouseWorldPos; ///<Mouse position on the worldspace XY plane
	float cumulativeMapDrag = 0;///<Ensures we don't drag for tiny amounts
	glm::vec3 lastMouseWorldPos;
	bool directionGraphicsOn = false;
	TCameraMode cameraMode = camNone;

	void createCursorObject();
	void setViewMode(TViewMode mode);
	void freeCam();
	void onPlayerDeath();
	void followCam(CEntity* ent);
	void adjustZoomScale(float delta);


private:


	void onNewMouseHex(CHex& mouseHex);
	int tigCall(int memberId) ;
	void updateCameraPosition();



	void initPalettes();

	void toggleDirectionGraphics();


	void freeCam(float x, float y);

	void fixedCam(float x, float y);

	void drawReticule();
	void removeDeadEntities();
	//void removeEntities();

	void zoomToFit();

//	CLevel* level;




	
	
	float dT; ///<Interval since last app loop.

	std::mt19937 randEngine;

	//CMapMaker mapMaker;





	THexList cursorPath;

	bool lineOfSight = false;


	
	TViewMode viewMode; ///<Camera mode, etc.



	CMapEdit mapEdit;

	float zoomAdjust = 0.0f;

	int highlitInvItem = 0;



	CEntity* pFollowCamEnt = NULL;

	glm::vec2 freeCamPos = { 0,0 };




	CRandLevel levelGen;


	//engine mode stuff
	CGameMode* mode; //<Current engine mode.

	std::unique_ptr<CGameMode> workingMode;
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
#define GLFW_KEY_LEFT_CONTROL       341

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

#define GLFW_KEY_0                  48
#define GLFW_KEY_1                  49
#define GLFW_KEY_2                  50
#define GLFW_KEY_3                  51
#define GLFW_KEY_4                  52
#define GLFW_KEY_5                  53
#define GLFW_KEY_6                  54
#define GLFW_KEY_7                  55
#define GLFW_KEY_8                  56
#define GLFW_KEY_9                  57

#define GLFW_MOD_SHIFT           0x0001
#define GLFW_MOD_CONTROL         0x0002
#define GLFW_MOD_ALT             0x0004
#define GLFW_MOD_SUPER           0x0008