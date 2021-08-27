#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "hex/hexArray.h"

#include "entity.h"

using TMapIt = std::unordered_multimap<CHex, CEntity*>::iterator;
using TRange = std::pair<TMapIt, TMapIt>;


/** Extend the hexArray to check for game entities when
	pathfinding. */
class CGameHexArray : public CHexArray {
public:
	CEntity* getEntityAt2(const CHex& hex);
	THexList findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive);
	void updateVisibility(THexList& visibleHexes, THexList& unvisibleHexes);
	void addEntity(TEntity entity, CHex& hex);
	void addExistingEntity(CEntity* entity, CHex& hex);
	void removeEntity(CEntity*  entity);

	void deleteEntity(CEntity& entity);

	void tidyEntityLists();
	
	void save(std::ostream& out);
	void load(std::istream& in);

	TEntities entities; ///<The grand list of entities in the map.
	
	bool entityListDirty = false;

	bool mapUpdated = false;
private:

};

class CMapHeader {
public:
	int width;
	int height;
};



