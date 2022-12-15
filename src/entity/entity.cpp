#include "entity.h"

#include <string>
#include <memory>

#include "utils\log.h"

#include "..\3Dtest\src\hexRenderer.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "../gameState.h"

#include "../hexRender/entityDraw.h"


const float rad360 = M_PI * 2;

unsigned int CEntity::nextId = 1;

CEntity::CEntity() {
	if (id == 0)
		id++;
	id = nextId++;

	transform = std::make_shared<CTransformCmp>(this);
	modelCmp = std::make_shared<CModelCmp>(this);
}

void CEntity::update(float dT) {
	//update various components here
	if (item)
		item->update(dT);
	if (transform)
		transform->update(dT);  //TO DO: may not need
	if (modelCmp)
		modelCmp->update(dT);
}



/** Set position using hex cube coordinates. */
void CEntity::setPosition(CHex& hex) {
	setPosition(cubeToWorldSpace(hex));
}
//TO DO: phase out the old hex-based usage!

void CEntity::setPosition(glm::vec3& worldPos) {
	hexPosition = worldSpaceToHex(worldPos);
	destination = hexPosition;

	glm::i32vec2 axial = hexPosition.getAxial();

	transform->setPos(worldPos);

	this->worldPos = worldPos;
	//TO DO: phase out entity worldPos!

	//buildWorldMatrix();
}



glm::vec3 CEntity::getPos()
{
	return transform->worldPos;
}

/** Set the rotation and facing direction of this object. */
void CEntity::setHexDirection(THexDir direction) {
	facing = direction;
	transform->setRotation(dirToAngle(direction));
	//buildWorldMatrix();
}


/** Construct this object's world matrix from its known position and rotation.*/
//void CEntity::buildWorldMatrix() {
//	modelCmp->translateAll(worldPos);
//	modelCmp->rotate(rotation);
//	//return;
//
//	//!!!!!!!!still need for missile currently:
//	glm::mat4 tmpMatrix = glm::translate(glm::mat4(1), worldPos);
//	tmpMatrix = glm::rotate(tmpMatrix, rotation, glm::vec3(0, 0, -1));
//	tmpMatrix = glm::scale(tmpMatrix, scale);
//	
//	//NB: we use a CW system for angles
//	for (auto& mesh : model.meshes)
//		mesh.matrix = tmpMatrix;
//	//Kludgy, but we will usually want to move all meshes.
//}

//void CEntity::updateMatrices(TModelData& model) {
//	model.matrix = glm::translate(glm::mat4(1), worldPos);
//
//	for (auto& childModel : model.subModels)
//		updateMatrices(childModel);
//}

std::tuple<float, glm::vec3> CEntity::collisionCheck(CEntity* e2) {
	//get bounding-sphere radii
	float radius1 = modelCmp->model.getRadius();
	float radius2 = e2->modelCmp->model.getRadius();
	
	//check for overlap
	float entDist = glm::distance(getPos(), e2->getPos());
	if (entDist > radius1 + radius2)
		return { 0, {0,0,0} };
	else {
		float collisionDist = entDist - radius2;
		glm::vec3 collisionNormal = glm::normalize(e2->getPos() - getPos());
		return { collisionDist, collisionNormal};
	}

}




/** True if entity on screen. */
bool CEntity::isOnScreen() {
	return hexRendr2.isOnScreen(getPos());
}


std::string CEntity::getShortDesc() {
	//TO DO: placeholder! Short description may not always = name

	std::string hotDesc = "\\h{ item " + std::to_string(id) + "}"
		+ name + "\\h";
	
	return hotDesc;
}







