#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in int colourId ;

uniform mat4 mvpMatrix;
uniform vec4 colourPalette[4];

out vec4 colourV; 



void main() {
	gl_Position = mvpMatrix * vec4(position,1);
	colourV = colourPalette[colourId];


}