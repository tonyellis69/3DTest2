#version 330
#include noise.lib

/** Convert the given world-space point to sample-space, and feed it to the geoemetry shader. */

layout(location = 0) in vec3 point; //A worlfd-space point somewhere on the surface.

uniform vec3 sampleOffset; //Position in sample space at the world-space origin.
uniform float sampleScale; //Converts world space to sample space.
uniform int mode; //What culling mode we're using


out TPointStatus {
	vec3 point; //The point we're investigating.
	float cull;  //1=0 cull, 0=keep;
} pointStatus;

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main () {
	
	vec3 samplePoint= sampleOffset + (point * sampleScale); //find the position in sample space of this point

	float mask;
	if (mode == 1)
		mask = fbm2D(3, (samplePoint.xz + vec2(5)) * 50.0f);
	if (mode == 2)
		 mask = fbm2D(3, samplePoint.xz * 100.0f); 

	
	pointStatus.point = point;
	pointStatus.cull = mask;

	
}