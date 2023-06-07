#include "playerModelCmp.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "..\hexRender\drawFunc.h"
#include "..\hexRender\multiDraw.h"

void CPlayerModelCmp::loadModel(CModel& pModel) {
	model = pModel;

	lowerMeshes.clear();
	upperMeshes.clear();
	lowerMasks.clear();
	upperMasks.clear();


	for (auto& mesh : model.meshes) {
		if (mesh.name == "footL" || mesh.name == "footR")
			lowerMeshes.push_back(&mesh);
		if (mesh.name == "body")
			upperMeshes.push_back(&mesh);
		if (mesh.name == "body_mask" )
			upperMasks.push_back(&mesh);
		if (mesh.name == "footL_mask" || mesh.name == "footR_mask")
			lowerMasks.push_back(&mesh);

		if (mesh.name == "footL")
			footL = &mesh;
		if (mesh.name == "footR")
			footR = &mesh;
		if (mesh.name == "footL_mask")
			footLmask = &mesh;
		if (mesh.name == "footR_mask")
			footRmask = &mesh;
	}

	thisEntity->collider->boundingRadius = glm::length(model.extents.furthestVert);

}

void CPlayerModelCmp::draw(CHexRender& hexRender) {
	drawFn->draw(hexRender);
}

void CPlayerModelCmp::initDrawFn() {

	auto fn = (CMultiDraw*)drawFn.get();

	for (auto& mesh : model.meshes) {
		if (mesh.name == "footL" || mesh.name == "footR")
			fn->lowerMeshes.push_back(&mesh);
		if (mesh.name == "body" )
			fn->upperMeshes.push_back(&mesh);
		if (mesh.name == "body_mask" )
			fn->upperMask = &mesh;
		if (mesh.name == "footL_mask" || mesh.name == "footR_mask")
			fn->lowerMasks.push_back(&mesh);
	}

	fn->model = &model;
}

void CPlayerModelCmp::update(float dT) {
	auto transform = thisEntity->transform;
	translateAll(transform->worldPos);
	rotateUpper(transform->upperBodyRotation);
	rotateLower(transform->rotation);
	retranslateLower(transform->walk);


}

void CPlayerModelCmp::translateAll(glm::vec3& pos) {
	tmpWorldM = glm::translate(glm::mat4(1), pos);
	for (auto& mesh : model.meshes)
		mesh.matrix = tmpWorldM;
}

void CPlayerModelCmp::rotateUpper(float angle) {
	glm::mat4 rotationM = glm::rotate(tmpWorldM, angle, glm::vec3(0, 0, -1));
	for (auto& mesh : upperMeshes)
		mesh->matrix = rotationM;
	for (auto& mesh : upperMasks)
		mesh->matrix = rotationM;
}

void CPlayerModelCmp::rotateLower(float angle) {
	glm::mat4 rotationM = glm::rotate(tmpWorldM, angle, glm::vec3(0, 0, -1));
	for (auto& mesh : lowerMeshes)
		mesh->matrix = rotationM;
	for (auto& mesh : lowerMasks)
		mesh->matrix = rotationM;
}

void CPlayerModelCmp::retranslateLower(glm::vec3& translate) {
	glm::mat4 Ltran = glm::translate(footL->matrix, -translate);
	footL->matrix = Ltran;
	footLmask->matrix = Ltran;
	glm::mat4 Rtran = glm::translate(footR->matrix,translate);
	footR->matrix = Rtran;
	footRmask->matrix = Rtran;
}
