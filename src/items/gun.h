#pragma once

#include "item.h"

class CGunType;

/** Generic fireable entity. Create different gun types
	with plug-ins. */
class CGun : public CItem {
public:
	CGun();
	void fire(float firingAngle);



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
};

class CSmallGun : public CGunType{
public:
	CSmallGun(CGun* gun) :  CGunType(gun) {};
	void fire(float firingAngle);
};