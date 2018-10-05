#version 330


in vec3 samplePoint0;

out vec4 FragColor;

uniform float frequency;

void main() {

	float x = samplePoint0.x * frequency;
	float y = samplePoint0.y * frequency;
	
	float distFromCenter = sqrt (x * x + y * y);
	float distFromSmallerSphere = distFromCenter - floor(distFromCenter);
	float distFromLargerSphere = 1.0 - distFromSmallerSphere;
	float nearestDist = min(distFromSmallerSphere, distFromLargerSphere);
	float value = 1.0 - (nearestDist * 4.0); // Puts it in the -1.0 to +1.0 range.
	
	value = value * 0.5 + 0.5;
	FragColor = vec4(value,value,value,1 );
	
};