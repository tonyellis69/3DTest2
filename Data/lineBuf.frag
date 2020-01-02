#version 330

in vec2 texCoordFrag;
out vec4 colour;

uniform sampler2D textureUnit;
uniform vec2 blockOffset;
uniform vec2 blockSize;



void main() {

	//colour = texture(textureUnit, blockOffset + blockSize * texCoordFrag.st );
	
	ivec2 coords = ivec2(blockOffset + blockSize * texCoordFrag.st );
	colour = texelFetch(textureUnit, coords,0);
	
	//colour.r = 1.0;

};
