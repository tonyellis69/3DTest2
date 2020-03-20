#pragma once

#include <random>

#include "Ivm.h"

#include "IMainApp.h"

#include "gameHexArray.h"

#include "mapMaker.h"

#include "hex/hexRenderer.h"

#include "IHexWorld.h"

#include "gamehextObj.h"
#include "robot.h"
#include "playerHexObj.h"
#include "hexItem.h"
#include "groupItem.h"

#include "bolt.h"

#include "tigConst.h"

enum TTurnPhase {actionPhase, chooseActionPhase, playerChoosePhase};

/** A class encapsulating the hex-based representation of the world. */
class IMainApp;
class CHexWorld : public IhexRendererCallback, public IHexWorld,
	public CTigObjptr {
public:
	CHexWorld();
	void setMainApp(IMainApp* pApp);
	void setVM(Ivm* pVM);
	void addMesh(const std::string& name, CMesh& mesh);
	void makeMap(ITigObj* tigMap);
	void start();
	void moveCamera(glm::vec3& direction);
	void beginRightClickAction();
	void beginLeftClickAction();
	void onKeyDown(int key, long mod);
	void setPlayerShield(THexDir direction);
	void onMouseWheel(float delta);
	void onMouseMove(int x, int y, int key);
	void draw();
	void setAspectRatio(glm::vec2& ratio);
	void update(float dt);

private:
	THexList calcPath(CHex& start, CHex& end);
	CGameHexObj* getEntityAt(CHex& hex);
	bool isBlockerMovingTo(CHex& hex);
	CHex getPlayerPosition();
	CHex getPlayerDestination();
	void temporaryCreateHexObjects();
	void onNewMouseHex(CHex& mouseHex);
	THexList* getPlayerPath();
	CGameHexObj* getPlayerObj();

	CHexObject* getCursorObj();



	void chooseActions();
	void startActionPhase();

	bool beginPlayerMove();

	bool resolvingGridObjActions();
	bool resolvingSerialActions();
	bool resolvingSimulActions();

	void tempPopulateMap();

	int tigCall(int memberId) ;

	void dropItem(CGameHexObj* item, CHex& location);

	CGroupItem* createGroupItem();

	void removeEntity(CGameHexObj& entity);
	void removeGridObj(CGridObj& gridObj);

	CGameHexObj* getItemAt(CHex& position);

	void tempGetGroupItem(int itemNo);

	void removeDeletedEntities();

	CGridObj* createBolt();

	void addToSerialActions(CGameHexObj* entity);


	CGameHexArray map;

	IMainApp* mainApp; ///<Pointer to app used for callbacks.
	CHexRenderer hexRenderer;


	CPlayerObject* playerObj;
	CHexObject* hexCursor;
	CRobot* robot;
	CRobot* robot2;
	CHexItem* wrench;
	CHexItem* shield;
	CHexItem* blaster;


	TEntities entities; ///<Live objects in the hex world.
	//TEntities playerItems; ///<Items temporarily taken out of hex world by player
	TEntities serialActions; ///<Entities performing serial actions this round.
	TEntities simulActions; ///<Entities performing simultaneous actions this round.
	
	std::vector<CGridObj*> gridObjects;

	TTurnPhase turnPhase;

	float dT; ///<Interval since last app loop.

	std::mt19937 randEngine;

	CMapMaker mapMaker;

	Ivm* vm; ///<Interface to the virtual machine/
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