#pragma once

#include <string>

#include "glm/glm.hpp"

#include "renderer/buf.h"

struct THexDraw;
class CLineModel;
class CHex;




/** An interface class for using CHexRenderer. */
class IHexRenderer {
public:
	virtual void drawLineModel(CLineModel& lineModel) = 0;
	virtual CLineModel getLineModel(const std::string& name) = 0;
	virtual glm::i32vec2 worldPosToScreen(glm::vec3& worldPos) = 0;
	virtual void highlightHex(CHex& hex) = 0;
	virtual void updateFogBuffer() = 0;
};

