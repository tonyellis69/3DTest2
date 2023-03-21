#pragma once

enum TGameEvent{gameLevelChange, gamePlayerDeath};
class CGameEvent {
public:
	CGameEvent() {};
	TGameEvent type;
};
