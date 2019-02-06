#version 330

#include noise.lib
#line 5

in vec2 texCoord0;  

uniform sampler2D source; 
uniform int iterations = 3;
uniform int stages = 3;

out vec4 FragColour;

ivec3 shapes[9] = ivec3[9]( ivec3(0,0,0), ivec3(1,1,1), ivec3(0,0,0),
ivec3(1,1,1), ivec3(0,0,1), ivec3(0,0,0),							
ivec3(0,1,1), ivec3(0,1,0), ivec3(1,1,0));


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}


vec2 calcShapeDimensions(vec2 seed, float regularity) {
	vec2 size = random2(seed);
	
	//return mix(size * 0.5 + vec2(0.4,0.2), size *  0.1 + vec2(0.84,0.85), regularity);
	return mix(size * 0.5 + vec2(0.4,0.2), vec2(1.0), regularity);
}


void main() {  
	float finalShade = 0;
	vec2 TL;
	vec2 BR;
	for (int stage = 2; stage <= stages; stage++) {
		for (int i = 1; i <= iterations; i++) {
			//subdivide space
			vec2 point = (texCoord0 * (i) * stage) ;
	
			//create rectangle
			vec2 cellTR = floor(point);
			vec2 size = calcShapeDimensions(cellTR + vec2(1), 0);
			size *= 1.0 / stage;
			TL = (vec2(1) - size) * random2(size.yx  * i);
			BR = TL + size;
			
			
			//are we inside the rectangle?
			point = fract(point);
			vec2 inTL = step(TL,point);
			vec2 inBR = step(point,BR) ;
			float shade =  inTL.x * inTL.y * inBR.x * inBR.y;
			
			
			//replace blocks with shapes
			if ( i > 1 && stage > 1) {
				vec2 clipPoint = point - TL;
	
				//scale to rectangle
				ivec2 index = ivec2(clipPoint / (size  / 3));
			
				float shapeStyle = rand(cellTR);
				if (rand(cellTR.yx) > 0.75)
						index.xy = index.yx;
				
				index.y += int( step(0.85,shapeStyle) + step(0.95,shapeStyle) ) * 3;
				shade *= shapes[index.y][index.x];		
			}
			
		//if (rand(BR) <= 0.25)
		//		shade = 0;
			
			
			float tint = i * stage * (0.8 / (iterations * stages )) + 0.1;
			shade*= tint ;
			
			finalShade =  max(finalShade,shade);
			
		}	
	}
	

	
	FragColour = vec4(vec3(finalShade),1);
	//FragColour.r =  0.7 *  (step(.98, point.x) + step(.98, point.y));
}