#include "modelCmp.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "..\hexRender\drawFunc.h"
#include "..\hexRender\multiDraw.h"

CModelCmp::CModelCmp(CEntity* parent) : CEntityCmp(parent) {
}

void CModelCmp::loadModel(CModel& pModel) {
	if (pModel.meshes.empty()) {
		int fail = 1;
	}
	model = pModel;

	

}

void CModelCmp::draw(CHexRender& hexRender) {
	drawFn->draw(hexRender);
}

void CModelCmp::update(float dT) {
	auto transform = parentEntity->transform.get();
	translateAll(parentEntity->getPos());
	rotate(transform->rotation);
	scale(transform->scale);
}

void CModelCmp::initDrawFn() {

	auto fn = drawFn.get();

	fn->model = &model;
}

void CModelCmp::setPalette(std::vector<glm::vec4>& pal) {
	model.palette = pal;

}

void CModelCmp::translateAll(glm::vec3& pos) {
	tmpWorldM = glm::translate(glm::mat4(1), pos);
	for (auto& mesh : model.meshes)
		mesh.matrix = tmpWorldM;
}

void CModelCmp::rotate(float angle) {
	tmpWorldM = glm::rotate(tmpWorldM, angle, glm::vec3(0, 0, -1));
	for (auto& mesh : model.meshes)
		mesh.matrix = tmpWorldM;
}

void CModelCmp::scale(glm::vec3& scale) {
	tmpWorldM = glm::scale(tmpWorldM, scale);
	for (auto& mesh : model.meshes)
		mesh.matrix = tmpWorldM;
}

void CModelCmp::rotateUpper(float angle) {
	glm::mat4 rotationM = glm::rotate(tmpWorldM, angle, glm::vec3(0, 0, -1));
	//for (auto& mesh : upperMeshes)
	//	mesh->matrix = rotationM;
	//for (auto& mesh : upperMasks)
	//	mesh->matrix = rotationM;
}

void CModelCmp::rotateLower(float angle) {
	glm::mat4 rotationM = glm::rotate(tmpWorldM, angle, glm::vec3(0, 0, -1));
	//for (auto& mesh : lowerMeshes)
	//	mesh->matrix = rotationM;
	//for (auto& mesh : lowerMasks)
	//	mesh->matrix = rotationM;
}

void CModelCmp::retranslateLower(glm::vec3& translate) {
	//glm::mat4 Ltran = glm::translate(footL->matrix, -translate);
	//footL->matrix = Ltran;
	//footLmask->matrix = Ltran;
	//glm::mat4 Rtran = glm::translate(footR->matrix,translate);
	//footR->matrix = Rtran;
	//footRmask->matrix = Rtran;
}
