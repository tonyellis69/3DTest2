#include "smartWin.h"

int CSmartWin::getEntityNo(const std::string& str) {
	return 	std::stoi(str.substr(str.find_first_of("0123456789")).c_str());
}
