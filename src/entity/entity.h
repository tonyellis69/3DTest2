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
#include "aiCmp.h"


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
	void setPosition(glm::vec3& worldPos);
	glm::vec3 getPos();
	virtual void receiveDamage(CEntity& attacker, int damage) {};
	virtual void onSpawn();
	void destroyMe();
	void setParent(CEntity* parent);
	CEntity* getParent();

	template <typename T>
	void addComponent(std::shared_ptr<T> t) {
		//TODO: handle default components here

	}
	void addComponent(std::shared_ptr<CPhys> phys);
	void addAIComponent(std::shared_ptr<CAiCmp> ai);

	template <typename T>
	T* removeComponent() {
		//TODO: handle default components here
		return nullptr;
	}
	void removePhysComponent();


	float dT;


	//CHex hexPosition = CHex(-1); ///<Position on a hex grid in cube coordinates.


	//components
	//CPhys physics;
	
	bool isItem = false; //temp cludge!
	bool isRobot = false; //temp cludge!
	bool isPlayer = false;

	bool deleteMe = false;

	bool live = true; ///<Entities are ignored when false.

	int tmpId;

	TEntityType entityType;

	std::string name;
	unsigned int id; 

	std::string diagnostic;

	//bool toRemove = false;

	std::shared_ptr<CTransformCmp> transform;
	std::shared_ptr<CEntityCmp> item;
	std::shared_ptr<ColliderCmp> collider;
	std::shared_ptr<CModelCmp> modelCmp;
	std::shared_ptr<CAiCmp> ai;
	std::shared_ptr<CPhys> phys;



private:
	static unsigned int nextId;

	std::shared_ptr<CEntity> parentEntity;
};

using TEntity = std::shared_ptr<CEntity>;
using TEntities = std::vector<std::shared_ptr<CEntity>>;

using CEntities = std::vector<CEntity*>;
