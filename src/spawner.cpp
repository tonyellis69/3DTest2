#include "spawner.h"

#include "robot.h"
#include "missile.h"

#include "items/gun.h"

#include "gameGui.h"

#include "items/shield.h"
#include "items/item2.h"

#include "hexRender/solidDraw.h"



std::unordered_map<std::string, CModel> CSpawn::models;
std::unordered_map<std::string, std::vector<glm::vec4> >* CSpawn::pPalettes;

CMap* CSpawn::pMap;


TEntity CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CPlayerObject>();
	player->setModel(models[name]);
	player->setPalette(pPalettes->at("basic")); //was basic
	player->initDrawFn();
	player->setPosition(pos);
	player->collider = std::make_shared<ColliderCmp>(player.get());


	CEntity* equippedGun = gun("guntype1");
	player->setGun(equippedGun);
	player->addToInventory(equippedGun);

	CEntity* equippedArmour = armour("basicArmour");
	player->setArmour(equippedArmour);
	player->addToInventory(equippedArmour);
	player->name = "player";

	CEntity* equippedShield = shield("basicShield");
	CItemCmp* item = (CItemCmp*)equippedShield->item.get();
	item->setOwner(player.get());
	player->shield = equippedShield;


	pMap->addEntity(player);
	return player;
}

/** Create the given entity, and notify the registered callback-handler. */
TEntity CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CRobot>();
	robot->setModel(models["robot"]);
	robot->setPalette(pPalettes->at("basic"));
	robot->initDrawFn();
	robot->setPosition(pos);

	robot->collider = std::make_shared<ColliderCmp>(robot.get());

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
		robot->setState(robotWander3);
		robot->entityType = entMeleeBot;
	}
	else  
	{
		robot->setState(robotWander3);
		robot->entityType = entShootBot;
	}
	r++;


	robot->name = "robot";

	pMap->addEntity(robot);
	return robot;
}

TEntity CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	missile->setModel(models["bolt"]);
	missile->setPalette(pPalettes->at("basic"));
	missile->initDrawFn();

	missile->setPosition(pos, angle);

	//missile->linesetColourR(glm::vec4(0.3, 1, 0.3, 1));

	pMap->addEntity(missile);
	return missile;
}

TEntity CSpawn::explosion(const std::string& name, glm::vec3& pos, float scale) {
	auto explode = std::make_shared<CExplosion>(scale);
	explode->worldPos = pos;
	explode->setPalette(pPalettes->at("explosion"));
	explode->initDrawFn();
	pMap->addEntity(explode);
	return explode;
}


CEntity* CSpawn::gun(const std::string& name, glm::vec3& pos ) {
	auto gun = std::make_shared<CGun>();

	gun->setModel(models["gun"]);
	gun->setPalette(pPalettes->at("gun"));
	gun->initDrawFn();

	gun->model.meshes[0].colour = glm::vec4(0, 1, 1.0, 0.45f);
	if (pos != glm::vec3(0,0,0) )
		gun->setPosition(pos);

	gun->gunType = std::make_shared<CSmallGun>(gun.get());
	gun->name = "gun";

	pMap->addEntity(gun);
	return gun.get();
}

CEntity* CSpawn::armour(const std::string& name, glm::vec3& pos) {
	auto armour = std::make_shared<CArmour>();

	armour->setModel(models["armour"]);
	armour->setPalette(pPalettes->at("armour"));
	armour->initDrawFn();

	armour->model.meshes[0].colour = glm::vec4(1, 0, 1.0, 0.45f);
	if (pos != glm::vec3(0, 0, 0))
		armour->setPosition(pos);

	armour->armourType = std::make_shared<CBasicArmour>(armour.get());
	armour->name = "armour";

	pMap->addEntity(armour);
	return armour.get();
}

CEntity* CSpawn::shield(const std::string& name) {
	auto shieldEnt = std::make_shared<CEntity>();

	shieldEnt->setModel(models["solidHex"]);
	shieldEnt->setPalette(pPalettes->at("shield"));
	shieldEnt->drawFn = std::make_shared<CSolidDraw>(shieldEnt.get());

	shieldEnt->item = std::make_shared<CShieldComponent>(shieldEnt.get());
	shieldEnt->name = name;

	pMap->addEntity(shieldEnt);
	return shieldEnt.get();
}



