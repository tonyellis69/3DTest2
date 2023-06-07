#pragma once

#include <vector>

#include "component.h"

#include "../viewField.h"

enum TMoveDir {
	moveNone, moveEast, moveWest, moveNE, moveSE,
	moveSW, moveNW, moveNorth, moveSouth, moveNS2, moveNS2blocked
};

class CGun;
class CArmour;
class CItem;

class CPlayerC : public CDerivedC<CPlayerC> {
public:
	CPlayerC(CEntity* parent);
	void onAdd();
	void onRemove();
	void onSpawn();
	void onFireKey(bool pressed);

	void moveCommand(TMoveDir commandDir);

	void updateViewField();

	void dropItem(int entityNo);

	void update(float dT);

	void setMouseDir(glm::vec3& mouseVec);

	void addToInventory(CEntity* item);

	void setArmour(CEntity* armour);

	void setShield(CEntity* shield);


	void setGun(CEntity* gun);

	void startTurnCycle();
	void updateWalkCycle();


	bool dead = false;

	CGun* gun; ///<Currently equipped gun.
	CArmour* armour; ///<Currently worn armour;
	std::vector<CItem*> inventory; ///<Carried items.

	CEntity* shield = nullptr;
	int shieldId;

	CViewFieldCircle viewField;
	glm::vec3 mouseVec; ///<Relative direction of mousepointer.


	TMoveDir moveDir = moveNone; ///<Move direction ordered by player.
	TMoveDir oldMoveDir = moveNone;
	bool walkingBackwards = false;
	float walkCycle = 0; ///<Tracks walking animation.
	float footExtension = 0;
	float maxFootExtension = 0.25f;
	float turningCycle = 0;
	glm::vec3 oldWorldPos = { 0,0,0 };

};