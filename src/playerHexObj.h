#pragma once

#include "hexItem.h"
#include "actor.h"

#include "powerSupply.h"

#include "gameEvents.h"
#include "hexMsg.h"

#include "viewField.h"

#include "UI/gui.h"



/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : public CHexActor , public CGameEventSubject {
public:
	CPlayerObject();
	~CPlayerObject();
	void onFireKey(bool pressed);
	bool update(float dT);
	void hitTarget();
	void draw();
	void takeItem(CGameHexObj& item);
	void showInventory();
	void dropItem(int itemNo);
	void equipItem(int itemNo);
	void leftClickPowerMode();
	void onTurnBegin();
	void onTurnEnd();
	void onGetPlayerPos(CGetPlayerPos& msg);
	void onSetPlayerAction(CSetPlayerAction& msg);
	void onTakeItem(CTakeItem& msg);
	void onGetPlayerObj(CGetPlayerObj& msg);

	void onPlayerSeen(CPlayerSeen& msg);

	void deathRoutine();

	void receiveDamage(CGameHexObj& attacker, int damage);

	int getMissileDamage();

	void onMovedHex();

	void updateViewField();

	void moveCommand(TMoveDir dir);
	CHex startNorthSouthMove(TMoveDir dir);
	void onVerticalKeyRelease();
	void update2(float dT);

	void setTargetAngle(float angle);
	float getTargetAngle() { return targetAngle;  }

	TEntities playerItems; ///<Items temporarily taken out of hex world by player
	
	CPowerSupply* psu;

	CViewFieldCircle viewField;

	CGUIlabel2* APlabel;

private:
	void moveReal();

	int actionPoints;


	float playerMoveSpeed = 5.0f;

	bool northSouthKeyReleased = true;

	float targetAngle;

};