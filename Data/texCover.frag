#version 330

#include noise.lib
#line 5

in vec2 texCoord0;  

uniform sampler2D source; 
uniform int iterations = 100;
uniform float scale = 1;

out vec4 FragColour;



float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}


vec2 calcShapeDimensions(vec2 seed) {
	vec2 size = random2(seed);
	return size * 0.2 + vec2(0.30,0.25);
}


void main() {  
	float finalShade = 0;
	vec2 TL;
	vec2 BR;
	vec2 cellTR;
	vec2 point;
	vec2 adjTexCoord = texCoord0;
	for (int i = 1; i <= iterations; i++) {
		adjTexCoord += random2(vec2(i));  //initial random offset for this iteration
		adjTexCoord = fract(adjTexCoord); //ensure wraparound
		
		point = adjTexCoord * 3; //speed things up by drawing 3x3 rects at a time
		cellTR = floor(point);
		point = fract(point);
		
		//create rectangle
		vec2 size = calcShapeDimensions(cellTR.yx) * scale;
		TL =  random2(cellTR) * vec2(0.5);
		BR = TL + size;
		
		//are we inside the rectangle?
		vec2 inTL = step(TL,point);
		vec2 inBR = step(point,BR) ;
		float shade =  inTL.x * inTL.y * inBR.x * inBR.y;
		
		float tint;
		//tint = i *  (0.8 / iterations) + 0.1;
		tint = rand(cellTR * i) ;
		shade *= tint ;
		
		//if (shade > 0)
		//	finalShade = shade;
		finalShade = (step(0.0,shade) * shade) + step(shade,0.0) * finalShade;
	}
	FragColour = vec4(vec3(finalShade),1);
	//point = fract(adjTexCoord * 3);
	//FragColour.r =  0.7 *  (step(0.98,point.x) + step(0.98, point.y));
}