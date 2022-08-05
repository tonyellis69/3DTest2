#include "../windows/examWin.h"

#include "../gameTextWin.h"

#include "UI/gui2.h"


void CExamWin::showWin() {
	centreOnMouse();
	CSmartWin::showWin();
}

void CExamWin::onEvent(CEvent& e) {
	if (e.type == eMouseOff && UI.mouseNotIn("exam") )
		pWin->setVisible(false);
}
