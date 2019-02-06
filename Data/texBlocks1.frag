#version 330

#include noise.lib
#line 5

in vec2 texCoord0;  

uniform sampler2D source; 
uniform int steps = 8;


out vec4 FragColour;

ivec3 shapes[9] = ivec3[9]( ivec3(0,0,0), ivec3(1,1,1), ivec3(1,1,1),
							ivec3(1,1,1), ivec3(0,0,1), ivec3(0,0,1),							
							ivec3(0,1,1), ivec3(0,1,0), ivec3(1,1,0));


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}


vec2 calcShapeDimensions(vec2 seed) {
	vec2 size = random2(seed);
	return size *  0.1 + vec2(0.15,0.1);
	//return vec2(0.5);
}




void main() {  
	float finalShade = 0;
	for (int i = 1; i < 4; i++) {
		vec2 point = texCoord0 * (steps/i);
		vec2 cellTL =  floor(point);
		
		//add some jitter
		vec2 jitter2 = random2(cellTL.yx) * vec2(0.3,0.1) ;
		//point += jitter2;
		//cellTL += jitter2;
		
		/*if (mod(cellTL.y,2) == 0) {
		point.x = (texCoord0.x + 0.04) * steps;
		//point += jitter2;
		cellTL =  floor(point);
	}*/
		
		point += jitter2;
		
		
		point = fract(point);
		
		float shapeStyle = rand(cellTL);
		
		vec2 size = calcShapeDimensions(cellTL);
		
		

		

		//clip to BB
		vec2 bbTL = (1 - size) * 0.5;
		vec2 bbBR = 1 - bbTL ;
		vec2 clipPoint = point - bbTL;
		
		//scale to BB
		ivec2 index = ivec2(clipPoint / (size  / 3));
		
		
		//index = clamp(index,ivec2(0),ivec2(2));
		
		index.y += int( step(0.85,shapeStyle) + step(0.95,shapeStyle) ) * 3;
		
		
		float shade = shapes[index.y][index.x];
		
		//discard any points outside BB
		vec2 inTL = step(bbTL,point);
		vec2 inBR = step(point,bbBR) ;
		shade *=  inTL.x * inTL.y * inBR.x * inBR.y;
		
		//shade *= step(0.2,shapeStyle);
		//if (mod(cellTL.x,2) == 0)
		//	shade = 0;
		
		
		
		// point = texCoord0 * steps;
		///point = fract(point);
		//FragColour.r =  0.7 *  (step(.98, point.x) + step(.98, point.y));
		finalShade += (shade * 0.25 * i);
	}
	FragColour = vec4(vec3(finalShade),1);
}