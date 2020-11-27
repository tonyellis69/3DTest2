#pragma once

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "hex/hexArray.h"

//#include "gamehextObj.h"
#include "actor.h" //replaces the above


#include "messaging/messenger.h"

#include "robot.h" //temp for testBot!

	

using TMapIt = std::unordered_multimap<CHex, CGameHexObj*>::iterator;
using TRange = std::pair<TMapIt, TMapIt>;

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
	CHex findLineEnd(CHex& start, CHex& target);
	TRange getEntitiesAt(CHex& hex);
	CGameHexObj* getEntityAt(CHex& hex);
	CGameHexObj* getEntityClassAt(int classId, CHex& hex);
	CGameHexObj* getBlockingEntityAt(CHex& hex);
	CGameHexObj* getEntityNotSelf(CGameHexObj* self);

	CHexActor* getRobotAt(CHex& hex);

	CGameHexObj* getEntityAt2(const CHex& hex);


	void updateBlocking();
	void smartBlockClear( CHex& pos);

	bool lineOfSight(CHex& start, CHex& end);

	CHex findRandomHex(bool unblocked);



	void onGetTravelPath(CGetTravelPath& msg);
	void onActorBlockCheck(CFindActorBlock& msg);
	void onGetLineEnd(CGetLineEnd& msg);
	void onGetActorAt(CGetActorAt& msg);
	void onLineOfSight(CLineOfSight& msg);
	void onRandomHex(CRandomHex& msg);
	void onFindViewField(CCalcVisionField& msg);
	THexList findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive);
	void onFindViewField2(CCalcVisionField& msg);

	void updateVisibility(THexList& visibleHexes, THexList& unvisibleHexes);

	CHex getSegmentFirstHex(glm::vec3& A, glm::vec3& B);

	std::tuple<THexDir, glm::vec3> findSegmentExit(glm::vec3& A, glm::vec3& B, CHex& hex);


	void addEntity(TEntity entity, CHex& hex);
	void movingTo(CGameHexObj* entity, CHex& pos, CHex& dest);
	void movedTo(CGameHexObj* entity, CHex& oldHex, CHex& newHex);
	void removeEntity(CGameHexObj* entity);


	TEntities entities; ///<The grand list of entities in the map.
	std::vector<CHexActor*> actors; ///<Shortlist of entities who are actors

	CRobot* testBot;
	CRobot* testBot2;
	//TO DO: temp!!


private:

	std::unordered_multimap<CHex, CGameHexObj*, hex_hash> entityMap;

};

const int solidHex = 2;
const int emptyHex = 1;