#pragma once

#include "component.h"
#include "modelCmp.h"

#include "model.h"


class CHexRender;
class CDrawFunc;
class CPlayerModelCmp : public CModelCmp {
public:
	CPlayerModelCmp(CEntity* parent) : CModelCmp(parent) {}
	void loadModel(CModel& model);
	void draw(CHexRender& hexRender);
	void initDrawFn();
	void update(float dT);

	void translateAll(glm::vec3& translate);
	void rotateUpper(float angle);
	void rotateLower(float angle);
	void retranslateLower(glm::vec3& translate);


	std::vector<TModelMesh*> lowerMeshes;
	std::vector<TModelMesh*> upperMeshes;
	std::vector<TModelMesh*> lowerMasks;
	std::vector<TModelMesh*> upperMasks;

	TModelMesh* footL;
	TModelMesh* footLmask;
	TModelMesh* footR;
	TModelMesh* footRmask;

};