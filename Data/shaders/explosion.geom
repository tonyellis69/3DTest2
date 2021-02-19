#version 330

 layout(points) in;
 layout(triangle_strip, max_vertices = 4) out;
 
in quad {
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d; 
} vQuad[];
 
 void main() {
	
	gl_Position =  vQuad[0].a;
	EmitVertex();
	
	gl_Position =  vQuad[0].b;
	EmitVertex();
	
	gl_Position =  vQuad[0].c;
	EmitVertex();
	
	gl_Position =  vQuad[0].d;
	EmitVertex();
	EndPrimitive();
 
};