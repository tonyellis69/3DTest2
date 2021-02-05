#version 330

 layout(points) in;
 layout(triangle_strip, max_vertices = 4) out;
 
in squareData {
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
	
	uint content;
	float fog;
	float highlight;
} hex[];

out vec3 texCoord;
flat out float outHighlight;

const float halfSquare = 1.0f;

void main() {

		if ( (
		hex[0].content != uint(2) &&  hex[0].highlight != 1.0f) 
			/* ||	hex[0].fog == 1.0f*/ ) 
			//!!!prevents drawing of fogged solid hexes - may not need
				return;
		

		outHighlight = hex[0].highlight;
		
		gl_Position =  hex[0].a;
		texCoord = vec3(halfSquare,halfSquare,0);
		EmitVertex();
		gl_Position =  hex[0].b;
		texCoord = vec3(-halfSquare,halfSquare,0);
		EmitVertex();
		gl_Position =  hex[0].c;
		texCoord = vec3(halfSquare,-halfSquare,0);
		EmitVertex();
		gl_Position =  hex[0].d;
		texCoord = vec3(-halfSquare,-halfSquare,0);
		EmitVertex();
		EndPrimitive();
		
}