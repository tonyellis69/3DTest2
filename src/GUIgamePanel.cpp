#include "GUIgamePanel.h"

CGUIgamePanel::CGUIgamePanel(int x, int y, int w, int h) : CGUIpanel(x,y,w,h)
{
	applyStyleSheet();
}

void CGUIgamePanel::applyStyleSheet()
{
	setBackColour1(styleSheet->defaultBackgroundColour);
	setBackColour2(styleSheet->defaultBackgroundColour);
}
