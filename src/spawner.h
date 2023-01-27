#pragma once

#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <unordered_map>

#include "entity\entity.h"
#include "missile.h"
#include "robot.h"
#include "explosion.h"
#include "playerHexObj.h"

#include "buf2.h"
#include "renderer/mesh.h"
#include "model.h"

#include "gameHexArray.h"


/** A class to tidy away the creation of entities. */
class CSpawn {
public:
	static TEntity player(const std::string& name, glm::vec3& pos);
	static TEntity robot(const std::string& name, glm::vec3& pos);
	static TEntity missile(const std::string& name, glm::vec3& pos, float angle);
	static TEntity explosion(const std::string& name, glm::vec3& pos, float scale);
	static CEntity* gun(const std::string& name, glm::vec3& pos = glm::vec3( 0,0,0 ));
	static CEntity* armour(const std::string& name, glm::vec3& pos = glm::vec3(0, 0, 0));

	static CEntity* shield(const std::string& name);

	static TEntity drop(const std::string& name, glm::vec3& pos);



	static std::unordered_map<std::string, CModel> models;
	static std::unordered_map<std::string, std::vector<glm::vec4> >* pPalettes;

	static CMap* pMap; ///<The map we're spawning to.

};
	
using spawn = CSpawn;