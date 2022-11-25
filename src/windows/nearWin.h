#pragma once

#include <vector>


#include "smartWin.h"
#include "listen/listen.h"
#include "UI/guiBase2.h"

#include "../entity/entity.h"
#include "../items/item.h"

class CNearWin : public CSmartWin {
public:
	void update(float dT);
	void refresh();
	void removeItem(int itemNo);

	std::vector<CItem*> nearItems; ///<Nearby items to report

};