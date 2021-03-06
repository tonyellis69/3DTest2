#version 330

layout(location = 0) in ivec2 index;
in uint content;

uniform mat4 mvpMatrix;

uniform ivec2 gridSize;

uniform samplerBuffer fogTex;

out squareData {
	vec4 vert;
	vec4 bVert;
	vec4 cVert;
	vec4 dVert;
	vec4 eVert;
	vec4 fVert;
	
	float fog;
} outData;



const float hexWidth = sqrt(3.0f);
const float hexHeight = 2;


void main() {

	ivec2 intOrigin = ivec2(gridSize * 0.5f);
	ivec2 offset = index - intOrigin;

	vec3 position;
	position.x = (offset.x + (0.5f * (offset.y & 1))) * hexWidth;
	position.y = -offset.y * 1.5f;
		
	outData.vert = mvpMatrix * ( vec4(position,1) +  vec4(0,hexHeight * 0.5f,0,0) );
	outData.bVert = mvpMatrix * ( vec4(position,1) +  vec4(-hexWidth * 0.5f,hexHeight * 0.25f,0,0) );
	outData.cVert = mvpMatrix * ( vec4(position,1) +  vec4(-hexWidth * 0.5f,-hexHeight * 0.25f,0,0) );
	outData.dVert = mvpMatrix * ( vec4(position,1) +  vec4(0,-hexHeight * 0.5f,0,0) );
	outData.eVert = mvpMatrix * ( vec4(position,1) +  vec4(hexWidth * 0.5f,-hexHeight * 0.25f,0,0) );
	outData.fVert = mvpMatrix * ( vec4(position,1) +  vec4(hexWidth * 0.5f,hexHeight * 0.25f,0,0) );


	int fogIndex = (index.y * int(22)) + index.x;
	outData.fog = texelFetch(fogTex, fogIndex ).r;
}
