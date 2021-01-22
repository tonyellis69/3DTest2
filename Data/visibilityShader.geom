#version 330

 layout(points) in;
 layout(triangle_strip, max_vertices = 4) out;
 

in hexQuad {
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
	
	//uint content;
	float fog;
	float visibility;
	float highlight;

	vec3[6] neighbours;
	
	ivec3 cube;
	ivec2 index;
} hex[];

out vec3 texCoord;

flat out float fog;
flat out float visibility;
flat out vec3[6] neighbours;
flat out ivec3 cube;	
flat out ivec2 index;


void main() {

		if (hex[0].fog != 1.0f /*&& hex[0].visibility == 1.0f*/ ) 
				return;
			
		fog = hex[0].fog;
		visibility = hex[0].visibility;
		neighbours = hex[0].neighbours;
		cube = hex[0].cube;
		index = hex[0].index;
			
		gl_Position =  hex[0].a;
		texCoord = vec3(1,1,0);
		EmitVertex();
		gl_Position =  hex[0].b;
		texCoord = vec3(-1,1,0);
		EmitVertex();
		gl_Position =  hex[0].c;
		texCoord = vec3(1,-1,0);
		EmitVertex();
		gl_Position =  hex[0].d;
		texCoord = vec3(-1,-1,0);
		EmitVertex();
		EndPrimitive();
		
	
		
}