#version 330


in vec2 texCoord0;

out vec4 FragColour;

uniform sampler2D source; 
uniform sampler1D palette; 
uniform int selectedShade;
float falloff = 0.05;

void main() {
	vec4 src = texture(source,texCoord0);
	vec4 colour = texture(palette,src.r +0.008 ); //TO DO: why am I doing this?
	
	if (selectedShade != -1) {
		float shade = float(selectedShade)/255.0;
		if (src.r > shade -falloff && src.r < shade +falloff) {
			colour = vec4(1.0 - colour.r, 1.0 - colour.g, 1.0 - colour.b,1);
			//colour.r = 1.0;
		}
		
	}
	
	
	FragColour = colour;
	
};

