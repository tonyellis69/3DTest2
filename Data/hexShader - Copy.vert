#version 330

layout(location = 0) in ivec2 index;

uniform mat4 mvpMatrix;

uniform ivec2 gridSize;

uniform vec2 viewPort;

out squareData {
	vec4 vert;
	vec4 bVert;
	vec4 cVert;
	vec4 dVert;
	vec4 eVert;
	vec4 fVert;
	vec4 origin;
	
	vec2 lineCentre[6];

} outData;

const float d = 0.65f;
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
	outData.origin = mvpMatrix * vec4(position,1);
	
	outData.lineCentre[0] = 0.5*(outData.vert.xy / outData.vert.w + vec2(1, 1))*viewPort;
	outData.lineCentre[1] = 0.5*(outData.bVert.xy / outData.bVert.w + vec2(1, 1))*viewPort;
	outData.lineCentre[2] = 0.5*(outData.cVert.xy / outData.cVert.w + vec2(1, 1))*viewPort;
	outData.lineCentre[3] = 0.5*(outData.dVert.xy / outData.dVert.w + vec2(1, 1))*viewPort;
	outData.lineCentre[4] = 0.5*(outData.eVert.xy / outData.eVert.w + vec2(1, 1))*viewPort;
	outData.lineCentre[5] = 0.5*(outData.fVert.xy / outData.fVert.w + vec2(1, 1))*viewPort;
	

}
