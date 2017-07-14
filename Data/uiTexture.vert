#version 330

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;

out vec2 texCoord0;
uniform mat4 orthoMatrix; 



void main()
{
	gl_Position = orthoMatrix * vec4(vertex.xy,0,1);
	
	texCoord0 = texCoord;
	
};
