#version 330

uniform vec4 colour = vec4(0,1,0,1.0f);

in vec3 texCoord;

out vec4 outputColor;

const float lineWidth = 0.15f;


float calcHexDistance(vec2 p) {
     const float hexHalfWidth= (sqrt(3) * 0.5f);// + (lineWidth * 0.125f) ; //0.5;
     const vec2 s = vec2(1, 1.7320508);
     p = abs(p);
     return max(dot(p, s * .5), p.x) - hexHalfWidth;
}

void main() {
	float dist = calcHexDistance(texCoord.xy);
	
	if (dist >0)
		dist = -1;

	float delta = fwidth(dist * 0.5f);
	
	float s = smoothstep(-delta * 2,-delta,dist) ;//-  smoothstep(0,lineWidth * 0.5f,dist);
	
	outputColor = colour * s;
	
	

}
