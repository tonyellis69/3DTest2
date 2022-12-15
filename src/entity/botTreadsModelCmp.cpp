#include "botTreadsModelCmp.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "..\hexRender\drawFunc.h"
#include "..\hexRender\multiDraw.h"

void CBotTreadsModelCmp::loadModel(CModel& pModel) {
	model = pModel;
	lowerMeshes.clear();
	upperMeshes.clear();
	lowerMasks.clear();
	upperMasks.clear();


	for (auto& mesh : model.meshes) {
		if ( mesh.name == "robase" || mesh.name == "treads")
			lowerMeshes.push_back(&mesh);
		if ( mesh.name == "robody")
			upperMeshes.push_back(&mesh);
		if (mesh.name == "robody_mask")
			upperMasks.push_back(&mesh);
		if (mesh.name == "robase_mask")
			lowerMasks.push_back(&mesh);

		if (mesh.name == "treads")
			treads = &mesh;
	}
	parentEntity->collider->boundingRadius = glm::length(model.extents.furthestVert);
}

void CBotTreadsModelCmp::draw(CHexRender& hexRender) {
	drawFn->draw(hexRender);
}

void CBotTreadsModelCmp::initDrawFn() {

	auto fn = (CMultiDraw*)drawFn.get();

	for (auto& mesh : model.meshes) {
		if (mesh.name == "robase" || mesh.name == "treads")
			fn->lowerMeshes.push_back(&mesh);
		if (mesh.name == "robody")
			fn->upperMeshes.push_back(&mesh);
		if (mesh.name == "robody_mask")
			fn->upperMask = &mesh;
		if (mesh.name == "robase_mask")
			fn->lowerMasks.push_back(&mesh);


	}

	fn->model = &model;
}

void CBotTreadsModelCmp::update(float dT) {
	auto transform = parentEntity->transform.get();
	translateAll(parentEntity->getPos());
	rotateUpper(transform->upperBodyRotation);
	rotateLower(transform->rotation);
	retranslateLower(transform->walk);
}



void CBotTreadsModelCmp::translateAll(glm::vec3& pos) {
	tmpWorldM = glm::translate(glm::mat4(1), pos);
	for (auto& mesh : model.meshes)
		mesh.matrix = tmpWorldM;
}

void CBotTreadsModelCmp::rotateUpper(float angle) {
	glm::mat4 rotationM = glm::rotate(tmpWorldM, angle, glm::vec3(0, 0, -1));
	for (auto& mesh : upperMeshes)
		mesh->matrix = rotationM;
	for (auto& mesh : upperMasks)
		mesh->matrix = rotationM;
}

void CBotTreadsModelCmp::rotateLower(float angle) {
	glm::mat4 rotationM = glm::rotate(tmpWorldM, angle, glm::vec3(0, 0, -1));
	for (auto& mesh : lowerMeshes)
		mesh->matrix = rotationM;
	for (auto& mesh : lowerMasks)
		mesh->matrix = rotationM;
}

void CBotTreadsModelCmp::retranslateLower(glm::vec3& translate) {
	treads->matrix = glm::translate(treads->matrix, translate);
}
