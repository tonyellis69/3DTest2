#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;


uniform mat4 orthoMatrix; 
uniform vec3 samplePos;
uniform vec3 sampleSize;


out vec2 texCoord0;
out vec2 samplePoint0;

out vec3 samplePos0;
out vec3 sampleSize0;



void main()
{
	gl_Position = vec4(position,0,1);
	texCoord0 = texCoord;
	
	vec2 unitPos = position * 0.5 + 0.5;
	samplePoint0 = samplePos.xy + unitPos * sampleSize.xy;
	
	samplePos0 = samplePos;
	sampleSize0 = sampleSize;
	
};
