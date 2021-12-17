#pragma once

#define _USE_MATH_DEFINES //for cmath

#include "entity.h"

#include "gameEvents.h"
#include "hexMsg.h"
#include "viewField.h"

#include "items/gun.h"
#include "items/armour.h"

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
	void setModel(CModel& model);
	void buildWorldMatrix();
	void tmpKeyCB(int key);
	void tmpDrop();
	void dropItem(int entityNo);
	void tmpTake();
	void updateInventory();
	void onFireKey(bool pressed);
	void draw();

	void receiveDamage(CEntity& attacker, int damage);

	void onMovedHex();

	void updateViewField();

	void moveCommand(TMoveDir commandDir);


	void update(float dT);

	void setTargetAngle(float angle);
	void setRotation(glm::vec3& vec);
	glm::vec3 getRotation();
	void setUpperBodyRotation(float n);
	void setUpperBodyRotation(glm::vec3& vec);
	float getUpperBodyRotation();
	glm::vec3 getUpperBodyRotationVec();
	void setMouseDir(glm::vec3& mouseVec);
	void addToInventory(CEntity* item);
	void setGun(CEntity* gun);
	void setArmour(CEntity* armour);

	TEntities playerItems; ///<Items temporarily taken out of hex world by player
	
	//CPowerSupply* psu;

	CViewFieldCircle viewField;


	TMoveDir travelDir = moveNone;


	bool visible = false;
	bool dead = false;

	int hp = 10;

	CGun* gun; ///<Currently equipped gun.
	CArmour* armour; ///<Currently worn armour;
	std::vector<CItem*> inventory; ///<Carried items.

private:
	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);
	void startTurnCycle();
	void updateWalkCycle();
	void nearItemUpdate();



	float dT;

	float playerMoveSpeed = 5.0f;

	//float targetAngle;

	//std::vector<CItem*> inventory;
	//std::vector<CItem*> tmpFloorItems; //temp!
	int itemSelected = -1; //temp!
	bool inventoryOn = false;

	float visibilityCooldown = 0;

	TModelMesh* upperBody;
	float upperBodyRotation = 0;

	TModelMesh* leftFoot;
	TModelMesh* rightFoot;

	glm::vec3 mouseVec; ///<Relative direction of mousepointer.

	TMoveDir moveDir = moveNone; ///<Move direction ordered by player.
	TMoveDir oldMoveDir = moveNone;
	bool walkingBackwards = false;
	float walkCycle = 0; ///<Tracks walking animation.
	float footExtension = 0;
	float maxFootExtension = 0.25f; 
	float turningCycle = 0;

	std::vector<TEntity> nearItems; ///<Nearby items to report


};