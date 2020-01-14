#include "playerHexObj.h"

bool CPlayerObject::update(float dT) {
	CHex playerOldHex = hexPosition;

	bool result = CHexObject::update(dT);

	if (hexPosition != playerOldHex) {
		hexWorld->onPlayerTurnDoneCB();
	}

	return result;
}
