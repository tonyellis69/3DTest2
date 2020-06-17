#pragma once


enum TAutoPower { autoNone, autoCurrent, autoLow, autoHigh, autoBest  };

class CPowerUser {
public:
	CPowerUser() {
		userId = userIds++;
	}

	unsigned int powerId = 0;
	unsigned int userId;

	TAutoPower autoPower;

private:
	static unsigned int userIds;
};

