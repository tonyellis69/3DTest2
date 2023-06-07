#pragma once

#include "component.h"


class CAiCmp : public CDerivedC<CAiCmp> {
public:
	CAiCmp(CEntity* parent) : CDerivedC(parent) {}
	void onAdd();
	void onRemove();


};