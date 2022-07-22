#version 330

#define PI 3.1415926538

uniform vec4 colour;
uniform float channel;
uniform float smoothing;
uniform float solid;

in vec2 gsTexCoord;
in vec4 gsColour;
flat in vec2 lineA;
flat in vec2 lineB;
in float gsThickness;

in vec2 gsWinSize;
flat in int gsSegNo;

layout(location = 0) out vec4 channel0; //scenery
layout(location = 1) out vec4 channel1; //model
layout(location = 2) out vec4 channel2; //glow



float lineDist(vec2 segA, vec2 segB, vec2 pt) {
	vec2 ab = segB - segA;
	vec2 ac = pt - segA;
	vec2 bc = pt - segB;

	float e = dot(ab, ac);
	if (e <= 0.0f)
		return dot(ac, ac);
	float f = dot(ab, ab);
	if (e >= f)
		return dot(bc, bc);

	return dot(ac, ac) - e * e / f;
}

float lineDist2(vec2 segA, vec2 segB, vec2 pt) {
	vec2 ab = segB - segA;
	float t = dot(pt - segA, ab) / dot(ab, ab);
	if (t < 0.0f) 
		t = 0.0f;
	if (t > 1.0f) 
		t = 1.0f;
	vec2 np = segA + t * ab;
	return distance(pt,np);
}


void main() {


		//outputColor = vec4(1,1,1,1);
	
	
	float lateralFade =  abs(gsTexCoord.y - 0.5) / 0.5;  // puts in form 1 - 0 - 1, like a graph axis
	
	float narrowing = 0.8;//  0.4f;// 1.0f;// 0.8f; // 1 = use all line thickness; >1 = fall off before line edge, <1 = fall off after line edge
	
	float falloff = 1.0f; // 1 = linear; >1 = slower fade toward edge, <1 = faster fade toward edge;
	
	float line = 1- pow((lateralFade * narrowing),falloff); 
	
	if (channel == 0.0) {
		channel0 = mix(gsColour,colour,colour.a);  
		channel0.a *= line;
	}
	else {
		//channel1 = mix(gsColour,colour,colour.a);  //standard
		//channel1.a *= line;
		
		vec2 p = gl_FragCoord.xy;

		
		float dist = lineDist(lineA, lineB, p);
		float normDist = sqrt(dist) / gsThickness;
		
		normDist = lineDist2(lineA, lineB, p) / gsThickness;
		
		if (normDist < 0) {
			channel1 = vec4(0,1,0,1);
		
			return;
		}
		
		channel1 = vec4(vec3(1 - normDist),1);
		float col = 1 - normDist; //0 = edge, 1 = centre

		/* if (gsSegNo == 1) //stripe diagnostic
			channel1 = vec4(col,0,0,1);	
		else
			channel1 = vec4(0,0,col,1);	
		*/
		
		//col = pow(col,smoothing);
		col = smoothstep(smoothing,solid,col);
		//col = smoothstep(0.5,0.9,col);
		
		channel1 = vec4(vec3(1),col);

	}

}
