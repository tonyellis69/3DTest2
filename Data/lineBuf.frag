#version 330

in vec2 texCoordFrag;
out vec4 colour;

uniform sampler2D textureUnit;
uniform vec2 blockOffset;
uniform vec2 blockSize;



void main() {
	vec2 blockCoord = texCoordFrag * blockSize;
	blockCoord += blockOffset;
	colour = texture(textureUnit, blockOffset + blockSize * texCoordFrag.st );
	colour.r = 1.0;

};
