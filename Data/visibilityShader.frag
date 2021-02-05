#version 330

uniform vec4 colour = vec4(0.545f, 0.0f, 0.545f, 1);

const vec4 fogColour = vec4(0.0,0,1,0.25f); //vec4( 0.1,0.1,0.1,1 );
const vec4 haze = vec4(0.4,0.4,0.4,0.5);

in vec3 texCoord;
flat in float fog;
flat in float visibility;

flat in vec3[6] neighbours;

flat in ivec3 cube;
flat in ivec2 index;


out vec4 outputColor;

const float hexWidth = sqrt(3.0f);
const float hexHalfWidth= (sqrt(3) * 0.5f);
const float hexHeight = 2;

const float rad60 = 3.1415926538 / 3.0f;
const float rad30 = 3.1415926538 / 6.0f;
const float pi2 = 3.1415926538 * 2.0f;

const float nBoxes = 6.0f;

const float fallOffWidth = 0.2f;
	 
vec3[] moveVector3D = vec3[]( vec3(hexWidth,0,0), vec3(hexWidth * 0.5f,-1.5f,0), 
	vec3(-hexWidth * 0.5f,-1.5f,0), vec3(-hexWidth,0,0), vec3(-hexWidth * 0.5f,1.5f,0), vec3(hexWidth * 0.5f,1.5f,0) );
	


float calcHexDistance(vec2 p) {
	const vec2 s = vec2(0.5f, hexHalfWidth); //perpendicular from NE face to hex centre
    p = abs(p);
	return max(dot(p, s), p.x) - hexHalfWidth;
}

int angleToDir(vec2 p) {
	vec2 norm = normalize(p);
	float normAngle = atan(norm.y, norm.x) + rad30;
		
	//find which neighbour hexes we're pointing between
	normAngle = mod(normAngle + pi2, pi2);  //put in range 0 - 2PI
	int dir = int(floor(normAngle/rad60));
	dir = (6 - dir) % 6; //make ordering clockwise	
	return dir;
}

float getValueAt(vec2 p) {
	//are we outside the home hex?
	float dist = calcHexDistance(p);
	float hexValue = fog;
	
	if (dist > 0) { //find where we are in neighbour hex
		int neighbour = angleToDir(p);	
		vec2 neighbourOrigin = moveVector3D[neighbour].xy;
		p = neighbourOrigin - p;
		dist = -1; //calcHexDistance(p);
		hexValue = neighbours[neighbour].r;
		return neighbours[neighbour].r; //for now, just return straight value of neighbour hex
	}
	else
		dist += 0;//0.1f; //shrink fog inside home hex
	
	float value = 1.0f - step(0.01f,dist); //1 if inside hex, else 0
	
	return value * hexValue;
}


void main() {
	float dist = calcHexDistance(texCoord.xy);    //dist < 0 = inside hex
	
	
	float hexShape = 1.0f - step(0,dist);    //0.02f
	//hexShape now 1 everywhere inside hex, 0 outside
	
	//if fog, just draw fog.
	//if not fog, apply visibility as needed
	
	if (fog == 1.0f) {
		if (hexShape == 0)
			return;
		
		float density = 1;  //1 = full fog, 0 = no fog
		for (int neighbour = 0; neighbour < 6; neighbour++) {
			if (neighbours[neighbour].r == 1)
				continue;
			
			vec2 dir = normalize(moveVector3D[neighbour].xy);
			
			float projection = dot(dir, texCoord.xy);

			if (projection > (hexHalfWidth - fallOffWidth) ) { //some lack of density
				float fallOff = projection - (hexHalfWidth - fallOffWidth);
				fallOff = 1 - (fallOff / fallOffWidth);
				//1 = no fallOff, 0 = total fallOff
				
				fallOff =  pow(fallOff,0.25f);
			
				//fallOff = smoothstep(0,1,fallOff);
				
				density  = min(fallOff,density);

			}
			//else 
			//	fallOff = 0;
		
		}
		
		outputColor = mix(vec4(0),fogColour,density);
		
		
		
		return;
		
		//find local box
		vec2 local = texCoord.xy * nBoxes;
		vec2 boxOffset = fract(local);
		
		vec2 c00 = floor(local) / nBoxes;;
		vec2 c10 = c00 + vec2(1.0/nBoxes,0);
		vec2 c01 = c00 + vec2(0, 1.0f/nBoxes);
		vec2 c11 = c00 + vec2(1.0f/nBoxes);
		
		
		float a = mix(getValueAt(c00),getValueAt(c10),boxOffset.x);
		float b = mix(getValueAt(c01),getValueAt(c11),boxOffset.x);
		
		float C = mix(a,b,boxOffset.y);
		
		
		outputColor = vec4(C,0,0,1) * hexShape;
		

		
		return;
		
		
		
		
		
		outputColor = fogColour * hexShape;	
		
		//normalise the fragment coord
		vec2 norm = normalize(texCoord.xy);
		
		//convert it to an angle
		float fragAngle = atan(norm.y, norm.x) + rad30;
		
		//find which neighbour hexes we're pointing between
		fragAngle = mod(fragAngle + pi2, pi2) ;  //put in range 0 - 2PI
		int dir = int(floor(fragAngle/rad60));
		dir = (6 - dir) % 6; //make ordering clockwise

		 if (dir == 0)
			 outputColor = vec4(0,0,0,1);
		 if (dir == 1)
			 outputColor = vec4(1,0,0,1);
		// if (dir == 2)
			// outputColor = vec4(0,1,0,1);
		// if (dir == 3)
			// outputColor = vec4(0,0,1,1);
		// if (dir == 4)
			// outputColor = vec4(1,1,0,1);
		// if (dir == 5)
			// outputColor = vec4(0,1,1,1);
		

		


		
		
	}
	else {
			outputColor = haze * hexShape;	
	}
	
	

}
