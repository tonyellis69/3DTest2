#version 330

in vec2 texCoordFrag;
out vec4 colour;

uniform sampler2D textureUnit;
uniform vec2 tile;


void main()
{
	ivec2 texel = ivec2(gl_FragCoord.xy);
	vec4 colour1 = texture2D(textureUnit, texCoordFrag.st );
	//vec4 colour1 = texelFetch(textureUnit, texel,0 );
	
	
	colour = colour1;
};
