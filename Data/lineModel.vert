#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 vertColour;


out vColour {
	vec4 colour; 
} colour_out;

uniform mat4 mvpMatrix;

void main() {
	gl_Position = mvpMatrix * vec4(position,1);
	colour_out.colour = vertColour;

}
