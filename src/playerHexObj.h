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
	void setModel(TModelData& model);
	void buildWorldMatrix();
	void tmpKeyCB(int key);
	void onFireKey(bool pressed);
	void draw();

	void receiveDamage(CEntity& attacker, int damage);

	void onMovedHex();

	void updateViewField();

	void moveCommand(TMoveDir commandDir);


	void update(float dT);

	void setTargetAngle(float angle);
	void setUpperBodyRotation(float n);
	float getUpperBodyRotation();
	//float getTargetAngle() { return targetAngle;  }

	TEntities playerItems; ///<Items temporarily taken out of hex world by player
	
	//CPowerSupply* psu;

	CViewFieldCircle viewField;


	TMoveDir travelDir = moveNone;


	bool visible = false;
	bool dead = false;

	int hp = 3;

private:
	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);
//	void trackMouse();



	float dT;

	float playerMoveSpeed = 5.0f;

	//float targetAngle;

	std::vector<CItem*> inventory;
	std::vector<CItem*> tmpFloorItems; //temp!
	int itemSelected = -1; //temp!
	bool inventoryOn = false;

	float visibilityCooldown = 0;

	TModelData* upperBody;
	float upperBodyRotation = 0;
};