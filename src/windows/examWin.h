#pragma once

#include "smartWin.h"
#include "listen/listen.h"

class CExamWin : public CSmartWin {
public:
	CExamWin() {
		lis::subscribe(this);
	}
	virtual void showWin();
	void onEvent(CEvent& e);


};