#pragma once

#include "component.h"
#include "model.h"


class CHexRender;
class CDrawFunc;
class CModelCmp : public CDerivedC<CModelCmp> {
public:
	CModelCmp(CEntity* parent) : CDerivedC(parent) {}
	void onAdd();
	void onRemove();
	virtual void loadModel(CModel& model);
	virtual void draw(CHexRender& hexRender);
	virtual void update(float dT);
	virtual void initDrawFn();
	virtual void setPalette(std::vector<glm::vec4>& pal);
	void setVisible(bool onOff);

	virtual void translateAll(glm::vec3& translate);
	virtual void rotate(float angle);
	virtual void scale(glm::vec3& scale);
	virtual void rotateUpper(float angle);
	virtual void rotateLower(float angle);
	virtual void retranslateLower(glm::vec3& translate);

	void onSpawn();

	CModel model;
	std::shared_ptr<CDrawFunc> drawFn;

	glm::mat4 tmpWorldM;
};