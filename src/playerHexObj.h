#pragma once

#define _USE_MATH_DEFINES //for cmath

#include "entity.h"

#include "gameEvents.h"
#include "hexMsg.h"
#include "viewField.h"
#include "item.h"

enum TMoveDir {
	moveNone, moveEast, moveWest, moveNE, moveSE,
	moveSW, moveNW, moveNorth, moveSouth, moveNS2, moveNS2blocked
};

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : /*public CHexActor ,*/ public CEntity, public CGameEventSubject {
public:
	CPlayerObject();
	~CPlayerObject();
	void tmpKeyCB(int key);
	void onFireKey(bool pressed);
	void draw();

	void receiveDamage(CEntity& attacker, int damage);

	void onMovedHex();

	void updateViewField();

	void moveCommand(TMoveDir commandDir);


	void update(float dT);

	void setTargetAngle(float angle);
	float getTargetAngle() { return targetAngle;  }

	TEntities playerItems; ///<Items temporarily taken out of hex world by player
	
	//CPowerSupply* psu;

	CViewFieldCircle viewField;


	TMoveDir travelDir = moveNone;


	bool visible = false;
	bool dead = false;

	int hp = 3;

private:
	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);
	//void approachDestHex();


	float dT;

	float playerMoveSpeed = 5.0f;

	//bool northSouthKeyReleased = true;

	float targetAngle;

	std::vector<CItem*> inventory;
	std::vector<CItem*> tmpFloorItems; //temp!
	int itemSelected = -1; //temp!
	bool inventoryOn = false;

	float visibilityCooldown = 0;


};