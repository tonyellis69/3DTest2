#version 330

#include noise.lib

in vec2 texCoord0;  

uniform sampler2D source; 

uniform int depth = 3;
uniform float freq = 3;
uniform int octaves = 4;
uniform float variance = 0.15;
uniform int style = 1; //0 = flat colour. 1 = noise. 
uniform float clusters = 0.0;

out vec4 FragColour;


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}


vec2 calcDivisor(vec2 tl, vec2 br) {
	vec2 rand = random2( vec2( rand(tl),rand(br) ) );
	vec2 range = (br - tl) * variance;
	return (range * rand) + tl + ( ((br - tl) - range) * 0.5);
}



void main() {  
	vec2 rectTL = vec2(0,0);
	vec2 rectBR = vec2(1,1);
	vec2 divisor;
	
	for (int step = 0; step < depth*2; step++) {
		divisor = calcDivisor(rectTL,rectBR);
		
		int axis = step % 2;

		if (texCoord0[axis] > divisor[axis]) {
			rectTL[axis]  = divisor[axis];
		}
		else {
			rectBR[axis]  = divisor[axis];
		}

	}

	
	float shade = rand(rectTL);

	float hint = fbm2DclassicFreq(1,rectTL,5);
	hint = hint * 0.5 + 0.5;


	if (style == 0) {
		if (hint < clusters)
			shade = shade *  0.25;
		//else
			//shade = shade * 0.75 +  0.25;
	}


	if (style == 1) {
		shade = fbm2DclassicFreq(octaves,rectTL * 5 + texCoord0,freq);
		shade = shade * 0.5 + 0.5;
		if (hint < clusters )
			shade = shade *  0.25;
			//shade -= 0.25;

	}


	
	FragColour = vec4(vec3(shade),1);
	
}