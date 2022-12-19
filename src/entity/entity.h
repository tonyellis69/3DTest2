#pragma once

#define _USE_MATH_DEFINES //for cmath

#include <glm/glm.hpp>

#include "hex/hex.h"
#include "../physics/phys.h"

#include "model.h"

#include "component.h"
#include "transform.h"
#include "collider.h"
#include "modelCmp.h"



struct TFov { //describes a fov shape
	CHex A;
	CHex B;
	THexList arc;
};

enum TEntityType {entNull = 0, entPlayer = 1, entMissile, entExplode, 
	entMeleeBot, entShootBot, entGun};

/** The base class for objects in the game world.*/
class CDrawFunc;
class CEntity  {
public:
	CEntity();
	virtual ~CEntity() {}
	virtual void update(float dT);
	void setPosition(CHex& hex);
	void setPosition(glm::vec3& worldPos);
	//virtual void setBoundingRadius();
	glm::vec3 getPos();
	void setHexDirection(THexDir direction);
	THexDir getDirection() {
		return facing;
	}
	//virtual void buildWorldMatrix();

	//virtual void updateMatrices(TModelData& model);

	virtual void receiveDamage(CEntity& attacker, int damage) {};
	virtual void onMovedHex() {};
	virtual std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB) {
		return std::tuple<bool, glm::vec3>();
	}
	std::tuple<float, glm::vec3> collisionCheck(CEntity* e2);


	bool isOnScreen();
	
	virtual std::string getShortDesc();


	//virtual void setPalette(std::vector<glm::vec4>& palette);

	
	
	float dT;

	//CModel model;

	CHex hexPosition = CHex(-1); ///<Position on a hex grid in cube coordinates.
	THexDir facing = hexEast; ///<Direction entity is facing.
	//glm::vec3 worldPos = { 0,0,0 }; ///<Position in world space.
	//glm::vec3 oldWorldPos = { 0,0,0 };

	THexList travelPath; ///<Route for movement.
	CHex destination; ///<The hex we're travelling to.

	//components
	CPhys physics;
	
	bool isItem = false; //temp cludge!
	bool isRobot = false; //temp cludge!
	bool isPlayer = false;

	bool deleteMe = false;

	bool live = true; ///<Entities are ignored when false.

	int tmpId;

	TEntityType entityType;

	std::string name;
	unsigned int id; 

	//std::shared_ptr<CDrawFunc> drawFn;

	//std::vector<glm::vec4>* pPalette;

	std::string diagnostic;

	bool toRemove = false;

	std::shared_ptr<CTransformCmp> transform;
	std::shared_ptr<CEntityCmp> item;
	std::shared_ptr<ColliderCmp> collider;
	std::shared_ptr<CModelCmp> modelCmp;

	float rotation = 0.0f; ///<Angle of object's z-rotation in world space.
	glm::vec3 scale = glm::vec3(1);
protected:


private:
	static unsigned int nextId;
};

using TEntity = std::shared_ptr<CEntity>;
using TEntities = std::vector<std::shared_ptr<CEntity>>;

using CEntities = std::vector<CEntity*>;