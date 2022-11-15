#pragma once

#include "smartWin.h"
#include "listen/listen.h"
#include "UI/guiEvent.h"


class CExamWin : public CSmartWin {
public:
	CExamWin() {
		lis::subscribe<CGUIevent>(this);
	}
	virtual void showWin();
	void onEvent(CGUIevent& e);


};