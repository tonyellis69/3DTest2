#pragma once

#include "ITigObj.h"
#include "Ivm.h"



#include "hex/hexArray.h"

/** A class for creating maps of individual rooms or areas. */
class CMapMaker {
public:

	void attachVM(Ivm* pVM);
	void attachMapObject(ITigObj* mapObj);
	CHexArray createMap();


private:
	ITigObj* mapObj; ///<The Tig template for the map we're creating.
	Ivm* vm; ///<Interface to the Tig VM.

};