#version 400
#include noise.lib

layout(location = 0) in vec3 point; //A point somewhere in a chunk.

uniform float currentY; //The current world space y value we are investigating. 
uniform vec3 chunkSamplePosition; //Position in sample space of the chunk.
uniform float sampleScale; //Converts world space to sample space.

out vec3 newPoint; //the xzy point we output.

const float cubeSize = 2.5;


vec3 vertexInterp(float isolevel, vec3 v0, float l0, vec3 v1, float l1){ 
	return mix(v0, v1, (isolevel-l0)/(l1-l0)); 
}

void main () {
	//substitute currentY
	vec3 currentPoint = point; 
	currentPoint.y = currentY;
	
	
	//translate  point into sample space
	vec3 samplePoint = (currentPoint * sampleScale)  + chunkSamplePosition;
	
	//find the terrain value at that point
	float value = terrainValue(5, samplePoint);
	
	
	

	newPoint = point;
	
	
	if (value < 0.5) { //samplePoint is inside terrain, so check the next point up
		vec3 nextPoint = currentPoint;
		nextPoint.y +=  cubeSize;
		vec3 nextSamplePoint = samplePoint;
		nextSamplePoint.y += cubeSize * sampleScale;
		float nextValue = terrainValue(5, nextSamplePoint);
		
		if (nextValue >= 0.5) { //surface intersects these 2 points
			vec3 intersect = vertexInterp(0.5,currentPoint,value,nextPoint,nextValue);
			newPoint  = intersect;
		}
	}
	
	
}