#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "hex/hexArray.h"

#include "entity.h"
//#include "actor.h" //replaces the above


#include "messaging/messenger.h"
#include "gameMsg.h"
//#include "robot.h" //temp for testBot!

	

using TMapIt = std::unordered_multimap<CHex, CEntity*>::iterator;
using TRange = std::pair<TMapIt, TMapIt>;
//using TIntersections = std::unordered_map<CHex, glm::vec3, hex_hash>;
using TIntersections = std::vector<std::pair<CHex, glm::vec3>>;

/** Extend the hexArray to check for game entities when
	pathfinding. */
class CGameHexArray : public CHexArray, 
	public CMessenger{
public:
	CGameHexArray();
	~CGameHexArray();
	void setMessageHandlers();
	void setEntityList(TEntities* pEntities);
	bool fromToBlocked(CHex& current, CHex& hex);
	bool isEmpty(glm::i32vec2& hex);
	bool isFree(CHex& hex);
	bool isAvailable(CHex& hex);
	CHex findLineEnd(CHex& start, CHex& target);
	TRange getEntitiesAt(CHex& hex);
	CEntity* getEntityAt(CHex& hex);
	//CGameHexObj* getEntityClassAt(int classId, CHex& hex);
	//CGameHexObj* getBlockingEntityAt(CHex& hex);
	CEntity* getEntityNotSelf(CEntity* self);



	CEntity* getEntityAt2(const CHex& hex);


	void updateBlocking();
	//void smartBlockClear( CHex& pos);

	bool lineOfSight(CHex& start, CHex& end);

	CHex findRandomHex(bool unblocked);



	void onGetTravelPath(CGetTravelPath& msg);
	void onGetLineEnd(CGetLineEnd& msg);
	void onLineOfSight(CLineOfSight& msg);
	void onRandomHex(CRandomHex& msg);
	void onFindViewField(CCalcVisionField& msg);
	THexList findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive);
	void onFindViewField2(CCalcVisionField& msg);

	void updateVisibility(THexList& visibleHexes, THexList& unvisibleHexes);

	CHex getSegmentFirstHex(glm::vec3& A, glm::vec3& B);

	TIntersections getIntersectedHexes(glm::vec3& segA, glm::vec3& segB);

	std::tuple<THexDir, glm::vec3> findSegmentExit(glm::vec3 A, glm::vec3 B, CHex& hex);


	void addEntity(TEntity entity, CHex& hex);
	void addExistingEntity(CEntity* entity, CHex& hex);
	void removeEntity(CEntity*  entity);
	void movingTo(CEntity* entity, CHex& pos, CHex& dest);
	void movedTo(CEntity* entity, CHex& oldHex, CHex& newHex);

	void deleteEntity(CEntity& entity);

	void tidyEntityLists();

	TEntities entities; ///<The grand list of entities in the map.
	

	//CRobot* testBot;
	//CRobot* testBot2;
	//TO DO: temp!!


private:


	std::unordered_multimap<CHex, CEntity*, hex_hash> entityMap;
	
	bool entityListDirty = false;

};



