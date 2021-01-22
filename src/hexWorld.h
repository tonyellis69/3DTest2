#pragma once

#include <random>

#include "Ivm.h"

#include "gameHexArray.h"

#include "mapMaker.h"

#include "hexRenderer.h"

#include "robot.h"
#include "playerHexObj.h"
#include "hexItem.h"
#include "groupItem.h"

//#include "bolt.h"

#include "tigConst.h"

#include "UI/GUIlabel2.h"

//#include "messaging/events.h"
#include "gameEvents.h"
#include "messaging/messenger.h"

#include "door.h" //hopefully *temporary* 

#include "gameTextWin.h"

#include "hexMsg.h"

#include "qps.h"

#include "gameState.h"

enum TViewMode {gameView, devView};
  

/** A class encapsulating the hex-based representation of the world. */
class CHexWorld :  public CGameEventSubject,
	public CMessenger, public CTigObjptr {
public:
	CHexWorld();
	void setVM(Ivm* pVM);
	void addMesh(const std::string& name, const std::string& fileName);
	void makeMap(ITigObj* tigMap);
	void deleteMap();
	void startGame();
	void moveCamera(glm::vec3& direction);
	void rightClick();
	void leftClick();
	void onFireKey(bool released);
	void onKeyDown(int key, long mod);
	void onMouseWheel(float delta, int key);
	void onMouseMove(int x, int y, int key);
	void draw();
	void setAspectRatio(glm::vec2& ratio);
	void update(float dt);

	void onCtrlLMouse();

	void enterKeyDown();

	void toggleView();

	CGUIlabel2* hexPosLbl;

private:

	void createCursorObject();
	void onNewMouseHex(CHex& mouseHex);

	int tigCall(int memberId) ;

	void dropItem(CGameHexObj* item, CHex& location);

	CGroupItem* createGroupItem();

	CGameHexObj* getItemAt(CHex& position);

	void tempGetGroupItem(int itemNo);

	void onDropItem(CDropItem& msg);
	void onRemoveEntity(CRemoveEntity& msg);
	void onCreateGroupItem(CCreateGroupItem& msg);

	void onDiceRoll(CDiceRoll& msg);

	void onPlayerNewHex(CPlayerNewHex& msg);

	void onActorMovedHex(CActorMovedHex& msg);


	CGameHexObj* getPrimaryObjectAt(CHex& hex);

	void updateCameraPosition();

	void beginNewTurn();
	

	void killEntity(CGameHexObj* entity);

	void alertEntitiesInPlayerFov();

	void setViewMode(TViewMode mode);


	CGameHexArray* map;

	//CHexRenderer* hexRenderer;


	CPlayerObject* playerObj = NULL;
	CGameHexObj* hexCursor = NULL;;
	

	TEntities entitiesToDraw; ///<Quick bodge: same entities, diff order


	float dT; ///<Interval since last app loop.

	std::mt19937 randEngine;

	CMapMaker mapMaker;

	Ivm* vm; ///<Interface to the virtual machine/


	glm::i32vec2 mousePos;

	THexList cursorPath;


	

	bool powerMode = false;

	CQPS qps; ///<Quantum power system.

	bool lineOfSight = false;
	glm::vec3 mouseWorldPos; ///<Mouse position on the worldspace XY plane
	
	TViewMode viewMode; ///<Camera mode, etc.


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