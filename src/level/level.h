#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "hex/hexArray.h"

#include "..\entity\entity.h"

using TMapIt = std::unordered_multimap<CHex, CEntity*>::iterator;
using TRange = std::pair<TMapIt, TMapIt>;



class CLevel  {
public:
	void init(int x, int y) {
		hexArray.init(x, y);
	}
	THexList findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive);
	void updateVisibility(THexList& visibleHexes, THexList& unvisibleHexes);
	CHexArray* getHexArray() {
		return &hexArray;
	}
	void setHexArray(CHexArray* newArray) {
		hexArray = *newArray;
	}
	CHexElement& getHexOffset(int x, int y) {
		return hexArray.getHexOffset(x, y);
	}
	CHex indexToCube(glm::i32vec2& index) {
		return hexArray.indexToCube(index);
	}
	glm::i32vec2 getGridSize();
	
	bool entitiesToDelete = false;
	bool entitiesToKill = false;


	bool mapUpdated = false;

//private:
	CHexArray hexArray;

};

class CMapHeader {
public:
	int width;
	int height;
};



