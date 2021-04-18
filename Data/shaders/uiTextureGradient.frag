#version 330

in vec2 texCoord0;
out vec4 fragColour;

uniform sampler1D textureUnit;


void main()
{
	fragColour = texture(textureUnit, texCoord0.x);
	
};
