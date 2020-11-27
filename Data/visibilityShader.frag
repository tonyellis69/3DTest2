#version 330

uniform vec4 colour = vec4(0.545f, 0.0f, 0.545f, 1);

const vec4 uialmostBlack = vec4(0.7,0,0,1); //vec4( 0.1,0.1,0.1,1 );
const vec4 haze = vec4(0.4,0.4,0.4,0.5);

in vec3 texCoord;
flat in float fog;
flat in float visibility;

flat in vec3[6] neighbours;

flat in ivec3 cube;
flat in ivec2 index;
flat in ivec2[6] neighbourIndexes;

out vec4 outputColor;

const float hexWidth = sqrt(3.0f);
const float hexHalfWidth= (sqrt(3) * 0.5f);
	 
vec3[] moveVector3D = vec3[]( vec3(hexWidth,0,0), vec3(hexWidth * 0.5f,-1.5f,0), 
	vec3(-hexWidth * 0.5f,-1.5f,0), vec3(-hexWidth,0,0), vec3(-hexWidth * 0.5f,1.5f,0), vec3(hexWidth * 0.5f,1.5f,0) );



float calcHexDistance(vec2 p) {

     const vec2 s = vec2(1, 1.7320508) * 0.5f;
     p = abs(p);
     return max(dot(p, s), p.x) - hexHalfWidth;
}

void main() {
	float dist = calcHexDistance(texCoord.xy);    //dist < 0 = inside hex
	
	float hexShape = 1.0f - smoothstep(-0.05,-0.04,dist);   //dist = 1.0f - step(0,dist);
	//hexShape now 1 everywhere inside hex, 0 outside
	
	
	//if fog, just draw fog.
	//if not fog, apply visibility as needed
	
	if (fog == 1.0f) {
		outputColor = uialmostBlack * hexShape;	
	/*
	float nearest = 20;
	int nCount = 0;
	for (int n=0; n<6; n++) {
		if (neighbours[n].r == 0.0f) {
			vec3 neighbourDistance = abs(moveVector3D[n] - texCoord);
			//float shortest = max(neighbourDistance.x,neighbourDistance.y);
			//float shortest =  distance(texCoord,moveVector3D[n]);
			float shortest = neighbourDistance.x + neighbourDistance.y;
			nearest = min(shortest,nearest);
			nCount++;
		}
	}
		
/*	if (nearest < hexWidth ) { 
		nearest -= hexHalfWidth; //puts in range hexHalfWidth to 0
		outputColor.a = ( nearest) * hexShape;
		
	}*/
	
	/*****************************
	outputColor = vec4(0,0,0,1) * hexShape;	//black shouldn't happen
	
	if (nCount == 0) 
		outputColor = vec4(0.9,0.9,0.9,1) * hexShape;	//white if surrounded - no unfogged neighbours
	if (nCount == 1)
		outputColor = vec4(1,0,0,1) * hexShape;	 // red for 1 unfogged neighbours
	if (nCount == 2)
		outputColor = vec4(0,1,0,1) * hexShape;	 // green for 2 unfogged neighbours
	if (nCount == 3)
		outputColor = vec4(0,0,1,1) * hexShape;	 // blue for 3 unfogged neighbours
	if (nCount == 4)
		outputColor = vec4(1,0,1,1) * hexShape;	 // magenta for 4 unfogged neighbours
	if (nCount == 5)
		outputColor = vec4(1,1,0,1) * hexShape;	 // yellow for 5 unfogged neighbours
	if (nCount == 6)
		outputColor = vec4(0,1,1,1) * hexShape;	 // cyan for 6 unfogged neighbours
	
	
	float innerHexShape = 1.0f - step(-0.5f,dist);
	if (innerHexShape == 1.0f && fog == 1.0f && neighbours[2].r == 0.0f) {
		float n = neighbours[2].r;
		outputColor = vec4(n,n,n,1);
	
	
		//if (neighbourIndexes[2] == vec2(1,1))
		//if (cube == vec3(2,-3,1) )
		if (index == ivec2(0,1) )
			outputColor = vec4(0.7,0.2,0.6,1);
		
		
		
	}
		*/
		
	}
	else {
			outputColor = haze * hexShape;	
	}
	
	

}
