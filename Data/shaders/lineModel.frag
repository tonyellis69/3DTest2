#version 330

#define PI 3.1415926538

uniform vec4 colour;

in vec2 gsTexCoord;
in vec4 gsColour;
in float pass;


layout(location = 0) out vec4 outputColor;
layout(location = 1) out vec4 outputMask;

void main() {

	if (pass == 0.0) {
	outputColor = mix(gsColour,colour,colour.a);

	
	
	float lateralFade =  abs(gsTexCoord.y - 0.5) / 0.5;  // puts in form 1 - 0 - 1, like a graph axis
	
	float narrowing =  0.4f;// 1.0f;// 0.8f; // 1 = use all line thickness; >1 = fall off before line edge, <1 = fall off after line edge
	
	float falloff = 1.0f; // 1 = linear; >1 = slower fade toward edge, <1 = faster fade toward edge;
	
	float line = 1- pow((lateralFade * narrowing),falloff); 
	

	outputColor.a *= line;    //NB smoothing thin lines makes them flicker when scrolling!
	
	
	}
	else {
		outputMask = mix(gsColour,colour,colour.a);
		
		
	float lateralFade =  abs(gsTexCoord.y - 0.5) / 0.5;  // puts in form 1 - 0 - 1, like a graph axis
	
	float narrowing =  0.9f;// 1.0f;// 0.8f; // 1 = use all line thickness; >1 = fall off before line edge, <1 = fall off after line edge
	
	float falloff = 1.0f; // 1 = linear; >1 = slower fade toward edge, <1 = faster fade toward edge;
	
	float line = 1- pow((lateralFade * narrowing),falloff); 
	

	//outputMask.a *= line;    //NB smoothing thin lines makes them flicker when scrolling!
	
	}
	

}
