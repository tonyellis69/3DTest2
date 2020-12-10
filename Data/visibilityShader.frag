#version 330

uniform vec4 colour = vec4(0.545f, 0.0f, 0.545f, 1);

const vec4 uialmostBlack = vec4(0.7,0,0,1); //vec4( 0.1,0.1,0.1,1 );
const vec4 haze = vec4(0.4,0.4,0.4,0.5);

in vec3 texCoord;
flat in float fog;
flat in float visibility;

flat in vec3[9] lattice;

flat in ivec3 cube;
flat in ivec2 index;


out vec4 outputColor;

const float hexWidth = sqrt(3.0f);
const float hexHalfWidth= (sqrt(3) * 0.5f);
const float hexHeight = 2;

const float rad60 = 3.1415926538 / 3.0f;
	 
vec3[] moveVector3D = vec3[]( vec3(hexWidth,0,0), vec3(hexWidth * 0.5f,-1.5f,0), 
	vec3(-hexWidth * 0.5f,-1.5f,0), vec3(-hexWidth,0,0), vec3(-hexWidth * 0.5f,1.5f,0), vec3(hexWidth * 0.5f,1.5f,0) );
	

//we're sampling from this grid:
//c01 -- c11
//|		  |
//c00 -- c10	
	
struct TQuadrant {
	int c10;
	int c11;
	int c00;
	int c01;	
};

TQuadrant[] quadrants = TQuadrant[] (	TQuadrant(0,1,3,4),  TQuadrant(1,2,4,5), 	
										TQuadrant(3,4,6,7),  TQuadrant(4,5,7,8) ); //(8,7,5,4) 



float calcHexDistance(vec2 p) {

     const vec2 s = vec2(1, 1.7320508) * 0.5f;
     p = abs(p);
     return max(dot(p, s), p.x) - hexHalfWidth;
}

float cotan(float z) {
		return cos(z)/sin(z);
}

float csc(float z) {
		return 1.0f / tan(z);
}

void main() {
	float dist = calcHexDistance(texCoord.xy);    //dist < 0 = inside hex
	
	float hexShape = 1.0f - smoothstep(-0.05,-0.04,dist);   //dist = 1.0f - step(0,dist);
	//hexShape now 1 everywhere inside hex, 0 outside
	
	
	//if fog, just draw fog.
	//if not fog, apply visibility as needed
	
	
	
	if (fog == 1.0f) {
		outputColor = uialmostBlack * hexShape;	
		
		return;
		
		//normalise the fragment coord
		vec2 norm = vec2(texCoord.x * (1 /hexWidth), texCoord.y * (2.0f/3.0f) );
		

		//skew it to the lattice
		vec2 skewed = norm;
		//skewed.x = norm.x - (norm.y * cotan(rad60));
		//skewed.y = norm.y * csc(rad60);
		
	
		//find which quadrant it's in
		vec2 matrixIndex = step(0, skewed.xy );
		int flatIndex = int(matrixIndex.x + ((1.0f - matrixIndex.y) * 2.0f));
		TQuadrant quadrant = quadrants[flatIndex];
		
		
		float c00 = lattice[quadrant.c00].r;
		float c01 = lattice[quadrant.c01].r;
		float c10 = lattice[quadrant.c10].r;
		float c11 = lattice[quadrant.c11].r;
		

		//make frag pos relative to bottom left of quadrant
		vec2 quadrantXY = fract(skewed + vec2(1));
		
		//interpolate the quadrant values
		
		float a = mix(c00,c10,quadrantXY.x);
		float b = mix(c01,c11,quadrantXY.x);
		
		float c = mix(a,b,quadrantXY.y);
		
		//outputColor = vec4(hexCoord.y,0,0,1);
		//outputColor =  vec4(length(hexCoord.xy),0,0,1);
		outputColor.a = c * hexShape;
		
		outputColor = vec4(0,0,c,1);
		
		/*if (flatIndex == 0)
			outputColor = vec4(1,0,0,1);
		if (flatIndex == 1)
			outputColor = vec4(0,1,0,1);
		if (flatIndex == 2)
			outputColor = vec4(0,0,1,1);
		if (flatIndex == 3)
			outputColor = vec4(1,0,1,1);
		*/
		
		float analyse = quadrantXY.x;
		float analyseY = quadrantXY.y;
		//outputColor = vec4(analyse,0,0,1);
		float line = step(0.48,analyse) * (1 - step(0.51,analyse));
		//line += step(0.0,analyse) * (1 - step(0.02,analyse));
		//line += step(-0.51,analyse) * (1 - step(-0.49,analyse));
		line += step(0.98,analyse) * (1 - step(0.99 ,analyse));
		
		float yLine = step(0.0,analyseY) * (1 - step(0.02,analyseY));
		yLine += step(0.48,analyseY) * (1 - step(0.51 ,analyseY));
		
		outputColor += vec4(yLine,line, 0, 1);

		
		
	}
	else {
			outputColor = haze * hexShape;	
	}
	
	

}
