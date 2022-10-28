#pragma once

#include "item.h"

class CGunType;

/** Generic fireable entity. Create different gun types
	with plug-ins. */
class CGun : public CItem {
public:
	CGun();
	void fire(float firingAngle);
	void update(float dT);
	void drop();
	std::string getShortDesc();

	std::string getMenuTextInv();

	std::shared_ptr<CGunType> gunType = nullptr; 


};


//!!!Gun types to plug into a gun entity;
class CGunType {
public:
	CGunType(CGun* gun) {
		this->gun = gun;
	}
	virtual void fire(float firingAngle) {}

	CGun* gun;

	float fireRate = 0.25f;
	float lastFired = 0;
};

class CSmallGun : public CGunType{
public:
	CSmallGun(CGun* gun) :  CGunType(gun) {};
	void fire(float firingAngle);


};