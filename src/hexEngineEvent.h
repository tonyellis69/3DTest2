#pragma once


enum TEngEventAction { engEventNone, engGetRenderer };
class CHexRender;
class CEngineEvent {
public:
	CEngineEvent(TEngEventAction e) : action(e) {}
	TEngEventAction action;

	CHexRender* pHexRender;
};