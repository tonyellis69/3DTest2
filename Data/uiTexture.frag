#version 330

in vec2 texCoordFrag;

out vec4 colour;

uniform sampler2D textureUnit;
uniform vec2 tile;
uniform vec2 offset;

void main()
{
	vec4 colour1 = texture2D(textureUnit, texCoordFrag.st );
	
	colour = colour1;
};
