#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "BaseApp.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


//#include "..\VMtest\src\vm.h"
//#include "UI\GUIimage.h"

//#include "UI\GUIrichText.h"

#include "utils/log.h"

#include "hexEngine.h" 

#include "gameTextWin.h"


enum TAppMode { terrainMode, textMode, texGenMode, hexMode,
	physicsMode};



class C3DtestApp : public  CBaseApp  {
public:
	C3DtestApp();
	bool OnMouseWheelMsg(float xoffset, float yoffset);

	//void onPopupText(CSendText& msg);
	//void onSysMessage(CSysMsg& msg);

	~C3DtestApp();
	void onStart();

	void keyCheck();
	void mouseMove(int x, int y, int key);
	void onKeyDown( int key, long mod);
	void onKeyUp(int key, long mod);

	void draw();
	void Update();

	void GUImsg(int ctrlId, TGUImessage& msg);

;
	void HandleUImsg(CGUIbase& Control, CMessage& Message);


	void onResize(int width, int height);

	void addGameWindow(CGUIbase* gameWin);

	void initHexWorld();

	glm::i32vec2 getMousePos();




	std::string dataPath; ///<Filepath to the Data folder


	CHexEngine hexEngine; 

	TAppMode appMode;
	
	
	CGameTextWin* defencePopWin = NULL; ///<Handle for popup window showing defence details on mouseover
	CGameTextWin* statusPopWin = NULL; ///<Handle for popup window showing status details on mouseover

	//CGameTextWin* powerQueueWin; ///<Handle for power queue window.
	//CGameTextWin* combatLogWin; ///<Handle for combat log window.

	unsigned int moveKeyDown = false;
	float moveKeyChangeTimer = 0;
};

const unsigned int upKey = 1;
const unsigned int rightKey = 2;
const unsigned int downKey = 4;
const unsigned int leftKey = 8;

