#include "spawner.h"

#include "robot.h"
#include "missile.h"

#include "items/gun.h"

std::unordered_map< std::string, CBuf2> CSpawn::meshBufs;
std::unordered_map< std::string, TModelData> CSpawn::modelBufs;

CMap* CSpawn::pMap;


TEntity CSpawn::player(const std::string& name, glm::vec3& pos) {
	auto player = std::make_shared<CPlayerObject>();
	setEntityLineModel(player.get(), name);
	player->setPosition(pos);

	player->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

//	player->setGun( (CGun*) gun("guntype1").get());
	player->setArmour((CArmour*)armour("basicArmour").get());

	pMap->addEntity(player);
	return player;
}

/** Create the given entity, and notify the registered callback-handler. */
TEntity CSpawn::robot(const std::string& name, glm::vec3& pos) {
	auto robot = std::make_shared<CRobot>();
	setEntityLineModel(robot.get(), "robot");
	robot->setPosition(pos);

	if (name == "melee bot") {
		robot->setState(robotWander3);
		robot->entityType = entMeleeBot;
	}
	else if (name == "shooter bot") {
		robot->setState(robotWander3);
		robot->entityType = entShootBot;
	}


	robot->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

	pMap->addEntity(robot);
	return robot;
}

TEntity CSpawn::missile(const std::string& name, glm::vec3& pos, float angle) {
	auto missile = std::make_shared<CMissile>();
	setEntityLineModel(missile.get(), "bolt");
	missile->setPosition(pos, angle);

	missile->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

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
	gun->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));

	setEntityLineModel(gun.get(), "gun");
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


void CSpawn::setEntityLineModel(CEntity* entity, const std::string& modelName) {
	entity->lineModel.buffer2 = &meshBufs[modelName];
	entity->setModel(modelBufs[modelName]);
}



