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
	CHex findLineEnd(CHex& start, CHex& target);
	void moveEntity(CGameHexObj* entity, CHex& hex);
	void add(CGameHexObj* entity, CHex& hex);
	void removeFromMap(CGameHexObj* entity);
	TRange getEntitiesAt(CHex& hex);
	CGameHexObj* getEntityAt(CHex& hex);
	CGameHexObj* getEntityClassAt(int classId, CHex& hex);
	CGameHexObj* getBlockingEntityAt(CHex& hex);
	CGameHexObj* getEntityNotSelf(CGameHexObj* self);

	CHexActor* getRobotAt(CHex& hex);


	void updateBlocking();
	void smartBlockClear( CHex& pos);

	bool lineOfSight(CHex& start, CHex& end);

	CHex findRandomHex(bool unblocked);

	void addActor(CHexActor* actor, CHex& hex);

	void onGetTravelPath(CGetTravelPath& msg);
	void onMoveEntity(CMoveEntity& msg);
	void onActorBlockCheck(CFindActorBlock& msg);
	void onGetLineEnd(CGetLineEnd& msg);
	void onGetActorAt(CGetActorAt& msg);
	void onGetObjectAt(CGetObjectAt& msg);
	void onLineOfSight(CLineOfSight& msg);
	void onRandomHex(CRandomHex& msg);
	void onFindViewField(CCalcVisionField& msg);
	void onFindViewField2(CCalcVisionField& msg);
	void onUpdateFog(CUpdateFog& msg);


	TEntities entities; ///<The grand list of entities in the map.
	std::vector<CHexActor*> actors; ///<Shortlist of entities who are actors

	CRobot* testBot;
	//TO DO: temp!!

private:

	std::unordered_multimap<CHex, CGameHexObj*, hex_hash> entityMap;

};