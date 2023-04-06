#include "spawner.h"

//#include "robot.h"
#include "missile.h"

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


std::unordered_map<std::string, CModel> CSpawn::models;
std::unordered_map<std::string, std::vector<glm::vec4> >* CSpawn::pPalettes;

CLevel* CSpawn::pMap;


CEntity* CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CEntity>();

	player->collider = std::make_shared<ColliderCmp>(player.get());
	player->modelCmp = std::make_shared<CPlayerModelCmp>(player.get());
	player->modelCmp->loadModel(models[name]);
	player->modelCmp->setPalette(pPalettes->at("basic"));
	player->modelCmp->drawFn = std::make_shared<CMultiDraw>(player.get());
	player->modelCmp->initDrawFn();
	player->setPosition(pos);

	player->phys = std::make_shared <CPhys>(player.get(), 1.0f / 80.0f);

	player->addComponent(std::make_shared<CPlayerC>(player.get()));
	player->addComponent(std::make_shared<CPlayerHealthC>(player.get()));

	CEntity* equippedGun = gun("guntype1");
	player->playerC->setGun(equippedGun);
	player->playerC->addToInventory(equippedGun);

	CEntity* equippedArmour = armour("basicArmour");
	player->playerC->setArmour(equippedArmour);
	player->playerC->addToInventory(equippedArmour);
	player->name = "player";

	player->entityType = entPlayer;

	CEntity* equippedShield = shield("basicShield");
	player->playerC->setShield(equippedShield);
	//CItemCmp* item = (CItemCmp*)equippedShield->item.get();
	//item->setOwner(player.get());
	//player->shield = equippedShield;


	game.addEntity(player);
	player->init();
	return player.get();
}

/** Create the given entity, and notify the registered callback-handler. */
CEntity* CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CEntity>();
	robot->collider = std::make_shared<ColliderCmp>(robot.get());
	robot->modelCmp = std::make_shared<CBotTreadsModelCmp>(robot.get());
	robot->modelCmp->loadModel(models["robot"]);
	robot->modelCmp->setPalette(pPalettes->at("basic")); 
	robot->modelCmp->drawFn = std::make_shared<CMultiDraw>(robot.get());
	robot->modelCmp->initDrawFn();
	robot->ai = std::make_shared<CRoboWander>(robot.get());
	robot->phys = std::make_shared <CPhys>(robot.get(), 1.0f / 80.0f);

	robot->addComponent(std::make_shared<CBotHealthC>(robot.get()));
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
		robot->ai = std::make_shared<CRoboWander>(robot.get());
		robot->entityType = entMeleeBot;
	}
	else  
	{
		robot->ai = std::make_shared<CRoboWander>(robot.get()); 
		robot->entityType = entShootBot;
	}
	r++;


	robot->name = "robot";

	game.addEntity(robot);
	robot->init();
	return robot.get();
}

CEntity* CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	missile->collider = std::make_shared<CMissileColliderCmp>(missile.get());
	missile->collider->colliderType = missileCollider;
	missile->modelCmp->drawFn = std::make_shared<CEntityDraw>(missile.get());
	missile->modelCmp->loadModel(models["bolt"]);
	missile->modelCmp->setPalette(pPalettes->at("basic")); 
	missile->modelCmp->initDrawFn();
	missile->addComponent( std::make_shared <CPhys>(missile.get(), 1.0f / 80.0f) );
	missile->setPosition(pos,angle);

	game.addEntity(missile);
	missile->init();
	return missile.get();
}

CEntity* CSpawn::explosion(const std::string& name, glm::vec3& pos, float scale) {
	auto explode = std::make_shared<CExplosion>(scale);

	explode->modelCmp->drawFn = std::make_shared<CSplodeDraw>(explode.get());
	explode->modelCmp->setPalette(pPalettes->at("explosion")); 
	explode->modelCmp->initDrawFn();
	//explode->worldPos = pos;
	explode->transform->setPos(pos);

	game.addEntity(explode);
	explode->init();
	return explode.get();
}


CEntity* CSpawn::gun(const std::string& name, glm::vec3& pos ) {
	auto gun = std::make_shared<CGun>();

	gun->modelCmp->loadModel(models["gun"]);
	gun->modelCmp->drawFn = std::make_shared<CItemDraw>(gun.get());
	gun->modelCmp->setPalette(pPalettes->at("gun")); 
	gun->modelCmp->initDrawFn();

	gun->modelCmp->model.meshes[0].colour = glm::vec4(0, 1, 1.0, 0.45f);
	if (pos != glm::vec3(0,0,0) )
 		gun->setPosition(pos);

	gun->gunType = std::make_shared<CSmallGun>(gun.get());
	gun->name = "gun";

	game.addEntity(gun);
	gun->init();
	return gun.get();
}

CEntity* CSpawn::armour(const std::string& name, glm::vec3& pos) {
	auto armour = std::make_shared<CArmour>();

	armour->modelCmp->loadModel(models["armour"]);
	armour->modelCmp->drawFn = std::make_shared<CItemDraw>(armour.get());
	armour->modelCmp->setPalette(pPalettes->at("armour"));
	armour->modelCmp->initDrawFn();

	armour->modelCmp->model.meshes[0].colour = glm::vec4(1, 0, 1.0, 0.45f);
	if (pos != glm::vec3(0, 0, 0))
		armour->setPosition(pos);

	armour->armourType = std::make_shared<CBasicArmour>(armour.get());
	armour->name = "armour";

	game.addEntity(armour);
	armour->init();
	return armour.get();
}

CEntity* CSpawn::shield(const std::string& name) {
	auto shieldEnt = std::make_shared<CEntity>();

	shieldEnt->modelCmp->loadModel(models["solidHex"]);
	shieldEnt->modelCmp->setPalette(pPalettes->at("shield")); 
	shieldEnt->modelCmp->drawFn = std::make_shared<CSolidDraw>(shieldEnt.get());
	shieldEnt->modelCmp->initDrawFn();

	shieldEnt->item = std::make_shared<CShieldComponent>(shieldEnt.get());
	shieldEnt->name = name;

	game.addEntity(shieldEnt);
	shieldEnt->init();
	return shieldEnt.get();
}


CEntity* CSpawn::drop(const std::string& name, glm::vec3& pos) {
	auto drop = std::make_shared<CEntity>();
	drop->collider = std::make_shared<ColliderCmp>(drop.get());
	drop->collider->sceneryOnly = true;
	drop->modelCmp->loadModel(models["hex"]);
	drop->modelCmp->drawFn = std::make_shared<CEntityDraw>(drop.get());
	drop->modelCmp->setPalette(pPalettes->at("basic"));
	drop->modelCmp->initDrawFn();
	drop->transform->setScale(glm::vec3{ 0.05f });
	drop->setPosition(pos);
	drop->addAIComponent(std::make_shared<CDropAI>(drop.get()) );
	drop->addComponent( std::make_shared<CPhys>(drop.get(), 1.0f / 80.0f) );

	game.addEntity(drop);

	drop->init();
	return drop.get();
}
