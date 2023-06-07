#include "spawner.h"

//#include "robot.h"
#include "missile.h"
#include "explosion.h"

#include "items/gun.h"

#include "gameGui.h"

#include "items/shield.h"
#include "items/armour.h"
#include "items/item2.h"

#include "hexRender/solidDraw.h"
#include "hexRender/entityDraw.h"
#include "hexRender/splodeDraw.h"
#include "hexRender/itemDraw.h"


#include "entity/playerModelCmp.h"
#include "entity/botTreadsModelCmp.h"
#include "physics/phys.h"
#include "ai/dropAI.h"
#include "entity/sceneryCollider.h"
#include "entity/missileCollider.h"
#include "entity/playerCmp.h"
#include "entity/botHealthC.h"
#include "entity/playerHealthC.h"
#include "hexRender/multiDraw.h"

#include "gameState.h"

#include "roboState.h"


CEntity* CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CEntity>();
	player->addComponent<CTransformCmp>();
	player->addComponent<CPlayerModelCmp>();
	player->addComponent<ColliderCmp>();
	player->modelCmp->loadModel(game.models[name]);
	player->modelCmp->setPalette(game.pPalettes->at("basic"));
	player->modelCmp->drawFn = std::make_shared<CMultiDraw>(player.get());
	player->modelCmp->initDrawFn();
	player->setPosition(pos);

	player->addComponent<CPhys>(1.0f/80.0f);
	player->addComponent<CPlayerC>();
	player->addComponent<CPlayerHealthC>();

	CEntity* equippedGun = gun("guntype1");
	equippedGun->tmpId = 42;
	player->playerC->setGun(equippedGun);
	player->playerC->addToInventory(equippedGun);

	CEntity* equippedArmour = armour("basicArmour");
	player->playerC->setArmour(equippedArmour);
	player->playerC->addToInventory(equippedArmour);
	player->name = "player";

	player->entityType = entPlayer;

	CEntity* equippedShield = shield("basicShield");
	player->playerC->setShield(equippedShield);

	game.addEntity(player);
	player->onSpawn();
	return player.get();
}

/** Create the given entity, and notify the registered callback-handler. */
CEntity* CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CEntity>();
	robot->addComponent<CTransformCmp>();
	robot->addComponent<CBotTreadsModelCmp>();
	robot->addComponent<ColliderCmp>();

	robot->modelCmp->loadModel(game.models["robot"]);
	robot->modelCmp->setPalette(game.pPalettes->at("basic"));
	robot->modelCmp->drawFn = std::make_shared<CMultiDraw>(robot.get());
	robot->modelCmp->initDrawFn();

	robot->addComponent<CRoboWander>();
	robot->addComponent<CPhys>(1.0f / 80.0f);
	robot->addComponent<CBotHealthC>();

	robot->healthC->hp = 3;

	robot->setPosition(pos);

	robot->isRobot = true;

	//if (name == "melee bot") {
	//	robot->setState(robotWander3);
	//	robot->entityType = entMeleeBot;
	//}
	//else if (name == "shooter bot") {
	//	robot->setState(robotWander3);
	//	robot->entityType = entShootBot;
	//}
	// test kludge for random bot distribution
	static int r = 0;
	if (r % 3 == 0) {
		robot->addComponent<CRoboWander>();
		robot->entityType = entMeleeBot;
	}
	else
	{
		robot->addComponent<CRoboWander>();
		robot->entityType = entShootBot;
	}
	r++;


	robot->name = "robot";

	game.addEntity(robot);
	robot->onSpawn();
	return robot.get();
}

CEntity* CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	missile->addComponent<CTransformCmp>();
	missile->setPosition(pos, angle);

	missile->addComponent<CModelCmp>();
	missile->addComponent<CMissileColliderCmp>();
	missile->collider->colliderType = missileCollider;

	missile->modelCmp->drawFn = std::make_shared<CEntityDraw>(missile.get());
	missile->modelCmp->loadModel(game.models["bolt"]);
	missile->modelCmp->setPalette(game.pPalettes->at("basic"));
	missile->modelCmp->initDrawFn();
	missile->addComponent<CPhys>(1.0f / 80.0f);

	missile->name = "missile";

	game.addEntity(missile);
	missile->onSpawn();
	return missile.get();
}

CEntity* CSpawn::explosion(const std::string& name, glm::vec3& pos, float scale) {
	auto explode = std::make_shared<CExplosion>(scale);
	explode->addComponent<CTransformCmp>();
	explode->addComponent<CModelCmp>();
	explode->modelCmp->drawFn = std::make_shared<CSplodeDraw>(explode.get());
	explode->modelCmp->setPalette(game.pPalettes->at("explosion"));
	explode->modelCmp->initDrawFn();
	explode->transform->setPos(pos);

	explode->name = "explosion";

	game.addEntity(explode);
	explode->onSpawn();
	return explode.get();
}


CEntity* CSpawn::gun(const std::string& name, glm::vec3& pos ) {
	auto gun = std::make_shared<CGun>();
	gun->addComponent<CTransformCmp>();
	gun->addComponent<CModelCmp>();
	gun->modelCmp->loadModel(game.models["gun"]);
	gun->modelCmp->drawFn = std::make_shared<CItemDraw>(gun.get());
	gun->modelCmp->setPalette(game.pPalettes->at("gun"));
	gun->modelCmp->initDrawFn();

	gun->modelCmp->model.meshes[0].colour = glm::vec4(0, 1, 1.0, 0.45f);
	if (pos != glm::vec3(0, 0, 0))
		gun->setPosition(pos);

	gun->gunType = std::make_shared<CSmallGun>(gun.get());
	gun->name = "gun";

	game.addEntity(gun);
	gun->onSpawn();
	return gun.get();
}

CEntity* CSpawn::armour(const std::string& name, glm::vec3& pos) {
	auto armour = std::make_shared<CArmour>();
	armour->addComponent<CTransformCmp>();
	armour->addComponent<CModelCmp>();
	armour->modelCmp->loadModel(game.models["armour"]);
	armour->modelCmp->drawFn = std::make_shared<CItemDraw>(armour.get());
	armour->modelCmp->setPalette(game.pPalettes->at("armour"));
	armour->modelCmp->initDrawFn();

	armour->modelCmp->model.meshes[0].colour = glm::vec4(1, 0, 1.0, 0.45f);
	if (pos != glm::vec3(0, 0, 0))
		armour->setPosition(pos);

	armour->armourType = std::make_shared<CBasicArmour>(armour.get());
	armour->name = "armour";

	game.addEntity(armour);
	armour->onSpawn();
	return armour.get();
}

CEntity* CSpawn::shield(const std::string& name) {
	auto shieldEnt = std::make_shared<CEntity>();
	shieldEnt->addComponent<CTransformCmp>();
	shieldEnt->addComponent<CModelCmp>();
	shieldEnt->modelCmp->loadModel(game.models["solidHex"]);
	shieldEnt->modelCmp->setPalette(game.pPalettes->at("shield"));
	shieldEnt->modelCmp->drawFn = std::make_shared<CSolidDraw>(shieldEnt.get());
	shieldEnt->modelCmp->initDrawFn();

	shieldEnt->addComponent<CShieldComponent>();
	shieldEnt->name = name;

	game.addEntity(shieldEnt);
	shieldEnt->onSpawn();
	return shieldEnt.get();
}


CEntity* CSpawn::drop(const std::string& name, glm::vec3& pos) {
	auto drop = std::make_shared<CEntity>();
	drop->addComponent<ColliderCmp>();
	drop->collider->sceneryOnly = true;
	drop->addComponent<CModelCmp>();
	drop->modelCmp->loadModel(game.models["hex"]);
	drop->modelCmp->drawFn = std::make_shared<CEntityDraw>(drop.get());
	drop->modelCmp->setPalette(game.pPalettes->at("basic"));
	drop->modelCmp->initDrawFn();

	drop->addComponent<CTransformCmp>();
	drop->transform->setScale(glm::vec3{ 0.05f });
	drop->setPosition(pos);

	drop->addComponent<CDropAI>();
	drop->addComponent<CPhys>(1.0f / 80.0f);

	drop->name = "drop";

	game.addEntity(drop);

	drop->onSpawn();
	return drop.get();
}
