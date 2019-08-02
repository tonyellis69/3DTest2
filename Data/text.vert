#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 texCoord0;
out vec2 position0;
uniform mat4 orthoMatrix; 

void main()
{
	gl_Position = orthoMatrix * vec4(position,0,1);
	
	position0 = gl_Position.xy * vec2(0.5) + vec2(0.5);
	
	texCoord0 = texCoord;
	
};
