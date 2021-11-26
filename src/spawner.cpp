#include "spawner.h"

#include "robot.h"
#include "missile.h"

#include "items/gun.h"

std::unordered_map<std::string, CModel> CSpawn::models;

CMap* CSpawn::pMap;


TEntity CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CPlayerObject>();
	player->setModel(models[name]);
	player->setPosition(pos);
	player->setGun( (CGun*) gun("guntype1").get());
	player->setArmour((CArmour*)armour("basicArmour").get());

	pMap->addEntity(player);
	return player;
}

/** Create the given entity, and notify the registered callback-handler. */
TEntity CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CRobot>();
	robot->setModel(models["robot"]);
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

	pMap->addEntity(robot);
	return robot;
}

TEntity CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	missile->setModel(models["bolt"]);
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


TEntity CSpawn::gun(const std::string& name, glm::vec3& pos ) {
	auto gun = std::make_shared<CGun>();

	gun->setModel(models["gun"]);
	gun->model.meshes[0].colour = glm::vec4(0, 1, 1.0, 0.45f);
	if (pos != glm::vec3(0,0,0) )
		gun->setPosition(pos);

	gun->gunType = std::make_shared<CSmallGun>(gun.get());

	pMap->addEntity(gun);
	return gun;
}

TEntity CSpawn::armour(const std::string& name) {
	auto armour = std::make_shared<CArmour>();
	armour->armourType = std::make_shared<CBasicArmour>(armour.get());

	pMap->addEntity(armour);
	return armour;
}



