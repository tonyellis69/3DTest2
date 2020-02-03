#version 330

in vec2 texCoordFrag;
out vec4 colour;

uniform sampler2D textureUnit;
uniform vec2 blockOffset;
uniform vec2 blockOffset2;
uniform vec2 blockSize;
uniform float alpha;


void main() {


	ivec2 coords = ivec2(blockOffset + blockSize * texCoordFrag.st );
	ivec2 coords2 = ivec2(blockOffset2 + blockSize * texCoordFrag.st );

	vec4 colour1 = texelFetch(textureUnit, coords,0);
	vec4 colour2 = texelFetch(textureUnit, coords2,0);

	colour = mix(colour1,colour2,alpha);

//	colour.a *= alpha;

};
