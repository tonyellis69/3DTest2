#pragma once

#define _USE_MATH_DEFINES //for cmath

#include "entity\entity.h"

//#include "gameEvents.h"
#include "hexMsg.h"
#include "viewField.h"

#include "items/gun.h"
#include "items/armour.h"

#include "items/item2.h"

enum TMoveDir {
	moveNone, moveEast, moveWest, moveNE, moveSE,
	moveSW, moveNW, moveNorth, moveSouth, moveNS2, moveNS2blocked
};

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : /*public CHexActor ,*/ public CEntity{ //}; , public CGameEventSubject{
public:
	CPlayerObject();
	~CPlayerObject();


	void dropItem(int entityNo);
	void onFireKey(bool pressed);

	void receiveDamage(CEntity& attacker, int damage);

	//void onMovedHex();

	void updateViewField();

	void moveCommand(TMoveDir commandDir);


	void update(float dT);

	void setTargetAngle(float angle);
	//void setRotation(glm::vec3& vec);
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

	int hp = 1;// 30;

	CGun* gun; ///<Currently equipped gun.
	CArmour* armour; ///<Currently worn armour;
	std::vector<CItem*> inventory; ///<Carried items.

	CEntity* shield = nullptr;
	//TODO: do I gain anything using a smartpointer here?

private:
	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);
	void startTurnCycle();
	void updateWalkCycle();
	void nearItemUpdate();



	float dT;

	//float playerMoveSpeed = 5.0f;

	//float targetAngle;

	//std::vector<CItem*> inventory;
	//std::vector<CItem*> tmpFloorItems; //temp!
	int itemSelected = -1; //temp!
	bool inventoryOn = false;

	float visibilityCooldown = 0;

	TModelMesh* upperBody;
	float upperBodyRotation = 0;
	TModelMesh* upperBodyMask;

	TModelMesh* leftFoot;
	TModelMesh* rightFoot;

	TModelMesh* leftFootMask;
	TModelMesh* rightFootMask;

	glm::vec3 mouseVec; ///<Relative direction of mousepointer.

	TMoveDir moveDir = moveNone; ///<Move direction ordered by player.
	TMoveDir oldMoveDir = moveNone;
	bool walkingBackwards = false;
	float walkCycle = 0; ///<Tracks walking animation.
	float footExtension = 0;
	float maxFootExtension = 0.25f; 
	float turningCycle = 0;
	glm::vec3 oldWorldPos = { 0,0,0 };

	std::vector<TEntity> nearItems; ///<Nearby items to report


};