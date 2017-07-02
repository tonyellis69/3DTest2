#version 330

layout(location = 0) in vec3 corner;
layout(location = 1) in vec3 opCorner;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix

out boxData {
	vec4 vert;
	vec4 bVert;
	vec4 cVert;
	vec4 dVert;
	
	vec4 eVert;
	vec4 fVert;
	vec4 gVert;
	vec4 hVert;
	
} outData;

void main() {
	outData.vert =    mvpMatrix * vec4(corner,1);
	outData.bVert =   mvpMatrix * ( vec4(corner,1) + vec4(opCorner.x,0,0,0));	
	outData.cVert =   mvpMatrix * ( vec4(corner,1) + vec4(opCorner.x,opCorner.y,0,0));
	outData.dVert =   mvpMatrix * ( vec4(corner,1) + vec4(0,opCorner.y,0,0));
	
	outData.eVert =   mvpMatrix * ( vec4(corner,1) + vec4(0,0,opCorner.z,0));
	outData.fVert =   mvpMatrix * ( vec4(corner,1) + vec4(opCorner.x,0,opCorner.z,0));	
	outData.gVert =   mvpMatrix * ( vec4(corner,1) + vec4(opCorner.x,opCorner.y,opCorner.z,0));
	outData.hVert =   mvpMatrix * ( vec4(corner,1) + vec4(0,opCorner.y,opCorner.z,0));
}
