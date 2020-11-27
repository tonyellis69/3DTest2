#version 330

layout(location = 0) in ivec2 index;
in uint content;


uniform mat4 mvpMatrix;

uniform ivec2 gridSize;

uniform samplerBuffer fogTex;

out squareData {
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
	
	uint content;
	float fog;
	float highlight;
	
} outData;


const float d = 1.0f;
const float hexWidth = sqrt(3.0f);
const float hexHeight = 2;


void main() {

	ivec2 intOrigin = ivec2(gridSize * 0.5f);
	ivec2 offset = index - intOrigin;

	vec3 position;
	position.x = (offset.x + (0.5f * (offset.y & 1))) * hexWidth;
	position.y = -offset.y * 1.5f;
		
		
	outData.a = mvpMatrix * ( vec4(position,1) +  vec4(d,d,0,0) );
	outData.b = mvpMatrix * ( vec4(position,1) +  vec4(-d,d,0,0) );
	outData.c = mvpMatrix * ( vec4(position,1) +  vec4(d,-d,0,0) );
	outData.d = mvpMatrix * ( vec4(position,1) +  vec4(-d,-d,0,0) );
	
	outData.content = content;
	
	int fogIndex = (index.y * int(22)) + index.x;
	outData.fog = texelFetch(fogTex, fogIndex ).r;
	outData.highlight = texelFetch(fogTex, fogIndex ).b;
}
