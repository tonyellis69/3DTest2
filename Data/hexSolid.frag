#version 330

uniform vec4 colour = vec4(0, 0.47f, 0.16f, 1);

in vec3 texCoord;
flat in float outHighlight;

out vec4 outputColor;


float calcHexDistance(vec2 p) {
     const float hexHalfWidth= (sqrt(3) * 0.5f);
     const vec2 s = vec2(1, 1.7320508);
     p = abs(p);
     return max(dot(p, s * .5), p.x) - hexHalfWidth;
}

void main() {
	float dist = calcHexDistance(texCoord.xy);    //dist < 0 = inside hex
	
	dist = 1.0f - step(0,dist);
	
	if (outHighlight == 1.0f)
		outputColor = vec4(0,0,0.6,1.0f) * dist;
	else
		outputColor = colour * dist;
	
	
}
