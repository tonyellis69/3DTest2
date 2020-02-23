#pragma once

#include "ITigObj.h"
#include "Ivm.h"



#include "gameHexArray.h"

/** A class for creating maps of individual rooms or areas. */
class CMapMaker {
public:

	void attachVM(Ivm* pVM);
	void attachMapObject(ITigObj* mapObj);
	CGameHexArray createMap();


private:
	ITigObj* mapObj; ///<The Tig template for the map we're creating.
	Ivm* vm; ///<Interface to the Tig VM.

};