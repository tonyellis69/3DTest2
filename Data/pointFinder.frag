#version 330


layout(location = 2) out vec4 fauxColour;

smooth in vec4 dummyColour;

void main() {
	
	fauxColour = dummyColour;
}