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
//#include "../items/armour.h"
//#include "../items/gun.h"
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
	virtual ~CEntity() {}
	virtual void update(float dT);
	void setPosition(glm::vec3& worldPos);
	glm::vec3 getPos();
	//virtual void receiveDamage(CEntity& attacker, int damage) {};
	virtual void init();
	void destroyMe();
	void setParent(CEntity* parent);
	CEntity* getParent();

	template <typename T, typename... Args>
	T* addComponentTest(Args... args) {
		auto comp = std::make_shared<T>(this,(args)...);


		addComponentTestSpec(comp);

		auto tmp2 = comp->testFunc();

		addComponentTestSpec2(tmp2);

		//if (typeid(CModelCmp) == typeid(tmp2)) {
		//	int a = 1;
		//}
		//else
		//{
		//	int a = 2;
		//}

		int id = comp->getUniqueID();

		components[id] = comp;

		return comp.get();
	}

	void addComponentTestSpec2(ColliderCmp* comp) {
		//collider = comp;
	}

	void addComponentTestSpec2(CItemCmp* comp) {
		//item = comp;
	}

	void addComponentTestSpec2(CPlayerC* comp) {
		//playerC = comp;
	}

	template <typename T>
	void addComponentTestSpec2(T t) {
		int a = 0;
	}



	template <typename T>
	void addComponentTestSpec(T t) {

	}

	void addComponentTestSpec(std::shared_ptr <CTransformCmp> comp) {
		transform = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CPhys> comp) {
		phys = comp;
	}

	void addComponentTestSpec(std::shared_ptr <ColliderCmp> comp) {
		//collider = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CModelCmp> comp) {
		modelCmp = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CPlayerModelCmp> comp) {
		modelCmp = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CPlayerC> comp) {
		playerC = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CPlayerHealthC> comp) {
		healthC = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CBotHealthC> comp) {
		healthC = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CBotTreadsModelCmp> comp) {
		modelCmp = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CRoboWander> comp) {
		ai = comp;
	}

	void addComponentTestSpec(std::shared_ptr <CShieldComponent> comp) {
	//	item = comp;
	}


	template <typename T>
	void addComponentTest2(std::shared_ptr<T> component) {
		auto tmp = typeid(component).name();

	}

	template <typename T>
	void addComponent(std::shared_ptr<T> t) {
		//TODO: handle default components here

	}
	void addComponent(std::shared_ptr<CPhys> phys);
	void addComponent(std::shared_ptr<CPlayerC> playerC);
	void addComponent(std::shared_ptr<CHealthC> healthC);
	void addComponent(std::shared_ptr<CPlayerHealthC> playerHealthC);
	void addComponent(std::shared_ptr<CBotHealthC> botHealthC);
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

	bool visible = true; //TODO: move to a component?

	int tmpId;

	TEntityType entityType;

	std::string name;
	unsigned int id; 

	std::string diagnostic;

	//bool toRemove = false;

	std::shared_ptr<CTransformCmp> transform;
	std::shared_ptr<CEntityCmp> item;
	std::shared_ptr<ColliderCmp> collider;
	//ColliderCmp* collider = nullptr;
	std::shared_ptr<CModelCmp> modelCmp;
	std::shared_ptr<CAiCmp> ai;
	std::shared_ptr<CPhys> phys;
	std::shared_ptr<CPlayerC> playerC;
	std::shared_ptr<CHealthC> healthC;

//	std::unordered_map<int, std::shared_ptr<CEntityCmp> > components;


private:
	static unsigned int nextId;

	std::shared_ptr<CEntity> parentEntity;
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