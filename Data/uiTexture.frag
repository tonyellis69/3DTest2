#version 330

in vec2 texCoordFrag;
out vec4 colour;

uniform sampler2D textureUnit;



void main()
{
	
	 colour = texture2D(textureUnit, texCoordFrag.st );
};
