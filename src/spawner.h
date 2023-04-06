#pragma once

#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <unordered_map>

#include "entity\entity.h"
#include "missile.h"
//#include "robot.h"
#include "explosion.h"
//#include "playerHexObj.h"

#include "buf2.h"
#include "renderer/mesh.h"
#include "model.h"

#include "level\level.h"


/** A class to tidy away the creation of entities. */
class CSpawn {
public:
	static CEntity* player(const std::string& name, glm::vec3& pos);
	static CEntity* robot(const std::string& name, glm::vec3& pos);
	static CEntity* missile(const std::string& name, glm::vec3& pos, float angle);
	static CEntity* explosion(const std::string& name, glm::vec3& pos, float scale);
	static CEntity* gun(const std::string& name, glm::vec3& pos = glm::vec3( 0,0,0 ));
	static CEntity* armour(const std::string& name, glm::vec3& pos = glm::vec3(0, 0, 0));

	static CEntity* shield(const std::string& name);

	static CEntity* drop(const std::string& name, glm::vec3& pos);



	static std::unordered_map<std::string, CModel> models;
	static std::unordered_map<std::string, std::vector<glm::vec4> >* pPalettes;

	static CLevel* pMap; ///<The map we're spawning to.

};
	
using spawn = CSpawn;