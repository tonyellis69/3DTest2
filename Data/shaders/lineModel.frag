#version 330

#define PI 3.1415926538

uniform vec4 colour;
uniform float channel;

in vec2 gsTexCoord;
in vec4 gsColour;



layout(location = 0) out vec4 channel0;
layout(location = 1) out vec4 channel1;



void main() {


		//outputColor = vec4(1,1,1,1);
	
	
	float lateralFade =  abs(gsTexCoord.y - 0.5) / 0.5;  // puts in form 1 - 0 - 1, like a graph axis
	
	float narrowing = 2;//  0.4f;// 1.0f;// 0.8f; // 1 = use all line thickness; >1 = fall off before line edge, <1 = fall off after line edge
	
	float falloff = 1.0f; // 1 = linear; >1 = slower fade toward edge, <1 = faster fade toward edge;
	
	float line = 1- pow((lateralFade * narrowing),falloff); 
	
	if (channel == 0.0) {
		channel0 = mix(gsColour,colour,colour.a);  
		channel0.a *= line;
	}
	else {
		channel1 = mix(gsColour,colour,colour.a);  
		channel1.a *= line;

	}

}
