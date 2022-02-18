#version 330

#define PI 3.1415926538

uniform vec4 colour;

in vec2 gsTexCoord;

in vec4 gsColour;

out vec4 outputColor;

void main() {

	outputColor = mix(gsColour,colour,colour.a);

	
	float lateralFade =  abs(gsTexCoord.y - 0.5) / 0.5;  // puts in form 1 - 0 - 1, like a graph axis
	
	float narrowing =  0.8f; // 1 = use all line thickness; >1 = fall off before line edge, <1 = fall off after line edge
	
	float falloff = 1.0f; // 1 = linear; >1 = slower fade toward edge, <1 = faster fade toward edge;
	
	float line = 1- pow((lateralFade * narrowing),falloff); 
	

	outputColor.a *= line;    //NB smoothing thin lines makes them flicker when scrolling!
	

}
