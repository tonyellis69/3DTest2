#version 330

layout(location = 0) in vec2 position;


//uniform mat4 matrix; 

out vec2 texCoord0;


void main()
{
	gl_Position = vec4(position,0,1);
	texCoord0 = position * 0.5 + 0.5;
	
};
