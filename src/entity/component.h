#pragma once

#define _USE_MATH_DEFINES

#include <string>

class CEntity;
class CEntityCmp  {
public:
	CEntityCmp(CEntity* parent) : thisEntity(parent) {}
	virtual void update(float dT) {}
	virtual void onAdd() {}
	virtual void onRemove() {}
	virtual void onSpawn() {} //TODO: call lateInit, it's more accurate 
	virtual CEntityCmp* onAdd(CEntity* parent) { return nullptr; }

	virtual int getUniqueID() const = 0;

	CEntity* thisEntity;

	float dT; //NB currently not updated by default;
};

class IDGenerator {
public:
	template <typename T>
	static int getID() {
		static int id = nextID_++;
		return id;
	}

private:
	static int nextID_;
};


template <typename T>
class CDerivedC : public CEntityCmp {
public:
	CDerivedC(CEntity* parent) : CEntityCmp(parent) {}

	int getUniqueID() const override {
		return IDGenerator::getID<T>();
	}

	virtual T* getClass() { return (T*)this; };
};

