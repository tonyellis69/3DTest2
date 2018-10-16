#version 330

layout(location = 0) in vec2 position;

uniform mat4 matrix; 
uniform vec3 samplePos;
uniform vec3 sampleSize;

out vec2 samplePoint0;
out vec2 texCoord0;


void main()
{
	gl_Position = vec4(position,0,1);
	
	vec2 unitPos = position * 0.5 + 0.5;
	samplePoint0 = samplePos.xy + unitPos * sampleSize.xy;
	//TO DO: remove samplePoint unless it can be found to be useful
	
	texCoord0 =  position * 0.5 + 0.5;
}
