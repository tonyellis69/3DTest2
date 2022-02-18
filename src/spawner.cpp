#include "spawner.h"

#include "robot.h"
#include "missile.h"

#include "items/gun.h"

#include "gameGui.h"

std::unordered_map<std::string, CModel> CSpawn::models;

CMap* CSpawn::pMap;


TEntity CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CPlayerObject>();
	player->setModel(models[name]);
	player->model.setBasePalette(std::vector<glm::vec4>({ {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,1,1} }) );
	player->initDrawFn();
	player->setPosition(pos);


	CEntity* equippedGun = gun("guntype1");
	player->setGun(equippedGun);
	player->addToInventory(equippedGun);

	CEntity* equippedArmour = armour("basicArmour");
	player->setArmour(equippedArmour);
	player->addToInventory(equippedArmour);
	player->name = "player";

	pMap->addEntity(player);
	return player;
}

/** Create the given entity, and notify the registered callback-handler. */
TEntity CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CRobot>();
	robot->setModel(models["robot"]);
	robot->model.setBasePalette(std::vector<glm::vec4>({ {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,1,1} }));
	robot->initDrawFn();
	robot->setPosition(pos);

	if (name == "melee bot") {
		robot->setState(robotWander3);
		robot->entityType = entMeleeBot;
	}
	else if (name == "shooter bot") {
		robot->setState(robotWander3);
		robot->entityType = entShootBot;
	}
	//robot->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));
	robot->name = "robot";

	pMap->addEntity(robot);
	return robot;
}

TEntity CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	missile->setModel(models["bolt"]);
	missile->model.setBasePalette(std::vector<glm::vec4>({ {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,1,1} }));
	missile->initDrawFn();

	missile->setPosition(pos, angle);

	//missile->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

	pMap->addEntity(missile);
	return missile;
}

TEntity CSpawn::explosion(const std::string& name, glm::vec3& pos, float scale) {
	auto explode = std::make_shared<CExplosion>(scale);
	explode->worldPos = pos;

	pMap->addEntity(explode);
	return explode;
}


CEntity* CSpawn::gun(const std::string& name, glm::vec3& pos ) {
	auto gun = std::make_shared<CGun>();

	gun->setModel(models["gun"]);
	gun->model.setBasePalette(std::vector<glm::vec4>({ {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,1,1} }));
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
	armour->model.setBasePalette(std::vector<glm::vec4>({ {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,1,1} }));
	armour->initDrawFn();

	armour->model.meshes[0].colour = glm::vec4(1, 0, 1.0, 0.45f);
	if (pos != glm::vec3(0, 0, 0))
		armour->setPosition(pos);

	armour->armourType = std::make_shared<CBasicArmour>(armour.get());
	armour->name = "armour";

	pMap->addEntity(armour);
	return armour.get();
}



