#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in int colourId ;


out vColour {
	vec4 colour; 
} colour_out;

uniform mat4 mvpMatrix;

uniform vec4 colourPalette[4] ;

void main() {
	gl_Position = mvpMatrix * vec4(position,1);
	if (colourPalette.length() == 0)
		colour_out.colour = vec4(1,0,0,1);
	else
		colour_out.colour = colourPalette[colourId];


}
