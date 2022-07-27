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

layout(location = 0) out vec4 channel0; 




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
		vec2 p = gl_FragCoord.xy;
		
		float normDist = lineDist2(lineA, lineB, p) / gsThickness;
		
		float col = 1 - normDist; //0 = edge, 1 = centre

		/* if (gsSegNo == 1) //stripe diagnostic
			channel1 = vec4(col,0,0,1);	
		else
			channel1 = vec4(0,0,col,1);	
		*/
		
		col = smoothstep(smoothing,solid,col);
		
		//above is smoothing code -  scrappable now we use MSAA. No obvious fps hit, however.
		
		
		channel0 = mix(gsColour,colour,colour.a);  
	
		
		//channel0 = vec4(channel0.xyz,col);


}
