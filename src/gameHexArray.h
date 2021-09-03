#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "hex/hexArray.h"

#include "entity.h"

using TMapIt = std::unordered_multimap<CHex, CEntity*>::iterator;
using TRange = std::pair<TMapIt, TMapIt>;



class CMap  {
public:
	void init(int x, int y) {
		hexArray.init(x, y);
	}
	CEntity* getEntityAt2(const CHex& hex);
	THexList findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive);
	void updateVisibility(THexList& visibleHexes, THexList& unvisibleHexes);
	void addEntity(TEntity entity, CHex& hex);
	void addExistingEntity(CEntity* entity, CHex& hex);
	void removeEntity(CEntity*  entity);
	void removeEntity(TEntity entity);
	CHexArray* getHexArray() {
		return &hexArray;
	}
	void setHexArray(CHexArray* newArray) {
		hexArray = *newArray;
	}
	CHexElement& getHexOffset(int x, int y) {
		return hexArray.getHexOffset(x, y);
	}
	void deleteEntity(CEntity& entity);

	void tidyEntityLists();
	
	void save(std::ostream& out);
	void load(std::istream& in);

	TEntities entities; ///<The grand list of entities in the map.
	
	bool entityListDirty = false;

	bool mapUpdated = false;

private:
	CHexArray hexArray;
};

class CMapHeader {
public:
	int width;
	int height;
};



