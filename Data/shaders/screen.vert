#version 330

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;




void main()
{
	gl_Position =  vec4(vertex.xy,0,1); 
	vTexCoord = texCoord;
	
};