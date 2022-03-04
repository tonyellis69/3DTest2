#version 330

uniform vec4 colour;

in vec4 colourV;

out vec4 outputColor;

void main() {

	outputColor = colourV;// colour;

}
