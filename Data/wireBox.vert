#version 330

layout(location = 0) in vec3 corner;
layout(location = 1) in vec3 opCorner;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix

out boxData {
	vec4 vert;
	vec4 opVert;
} outData;

void main() {
	outData.vert = mvpMatrix * vec4(corner,1);
	outData.opVert = vec4(opCorner,1);	
}
