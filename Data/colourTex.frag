#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform sampler1D palette; 

void main() {
	vec4 src = texture(source,texCoord0);
	vec4 colour = texture(palette,src.r +0.008 );
	
	//vec4 colour = texture(palette,clamp(src.r,0.006,0.95));
	
	//vec4 colour = texture(palette,src.r  );

	
	/*if (src.r == 0.0)
		colour = vec4(1,0,0,1);
	else
		colour = vec4(1,1,1,1);
		*/

	FragColour = colour;
	
};

