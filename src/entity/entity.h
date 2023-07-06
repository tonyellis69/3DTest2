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
#include "playerCmp.h"
#include "playerHealthC.h"
#include "botHealthC.h"
#include "playerModelCmp.h"
#include "botTreadsModelCmp.h"
#include "../ai/dropAI.h"
#include "../items/shield.h"
#include "../items/item2.h"
#include "../roboState.h"
#include <typeinfo>
#include <typeindex>


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
	virtual ~CEntity();
	virtual void update(float dT);
	void setPosition(glm::vec3& worldPos);
	glm::vec3 getPos();
	virtual void onSpawn();
	void destroyMe();
	void setParent(CEntity* parent);
	CEntity* getParent();
	std::shared_ptr<CEntity> getSmart();


	template <typename T, typename... Args>
	T* addComponent(Args... args) {
		auto comp = std::make_shared<T>(this,(args)...);
		auto classPtr = comp->getClass();

		int id = comp->getUniqueID();
		components[id] = comp;

		comp->onAdd();

		return comp.get();
	}

	void removeComponent(CEntityCmp* component);

	template <typename T>
	T* getComponent() {
		int id = IDGenerator::getID<T>();
		return (T*)components[id].get();
	}

	void onDestroy();



	float dT;

	
	bool isItem = false; //temp cludge!
	bool isRobot = false; //temp cludge!
	bool isPlayer = false;

	bool deleteMe = false;

	bool live = true; ///<Entities are ignored when false.

	bool visible = true; //TODO: move to a component?

	int tmpId;

	TEntityType entityType;

	std::string name;
	unsigned int id; 

	std::string diagnostic;


	CTransformCmp* transform = nullptr;
	CEntityCmp* item = nullptr;
	ColliderCmp* collider = nullptr;
	CModelCmp* modelCmp = nullptr;
	CAiCmp* ai = nullptr;
	CPhys* phys = nullptr;
	CPlayerC* playerC = nullptr;
	CHealthC* healthC = nullptr;

	std::unordered_map<int, std::shared_ptr<CEntityCmp> > components;


private:
	static unsigned int nextId;

	CEntity* parentEntity;
};

using TEntity = std::shared_ptr<CEntity>;
using TEntities = std::vector<std::shared_ptr<CEntity>>;

using CEntities = std::vector<CEntity*>;

enum TEnityEvent { entNone, entAdd, entRemove };
class CEntityEvent {
public:
	TEnityEvent eventType = entNone;
	CEntity* entity;
};