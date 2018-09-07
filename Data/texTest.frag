#version 330


in vec2 texCoord0;
out vec4 FragColor;


//uniform vec4 textColour;

void main() {
	
	FragColor = vec4(texCoord0.st,0,1.0 );
	//FragColor = vec4(1,0,0,1.0 );

};