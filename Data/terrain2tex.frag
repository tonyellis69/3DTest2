#version 330

//layout(origin_upper_left) 
//gl_FragCoord;

layout(location = 0) out vec4 colour;

uniform vec2 nwSampleCorner; //The position in sample space of the nw corner of our terrain.
uniform float pixelScale; //ratio of sample space to screen space.

#include noise.lib



void main() {
	vec2 samplePoint = nwSampleCorner + (gl_FragCoord.xy  * pixelScale);
		
	float noise = terrainValue(5, vec3(samplePoint.x,0,samplePoint.y)); 
	
	noise = (noise * 0.5f) + 0.5f; //put in 0-1 range
	
	colour = vec4(noise,noise,noise,1);
}