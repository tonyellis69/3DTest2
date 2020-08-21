#version 330

 layout(points) in;
 layout(triangle_strip, max_vertices = 4) out;
 
in squareData {
	vec4 vert;
	vec4 bVert;
	vec4 cVert;
	vec4 dVert;
} hex[];

out vec3 texCoord;

const float halfSquare = 1.0f;

void main() {
		
		gl_Position =  hex[0].vert;
		texCoord = vec3(halfSquare,halfSquare,0);
		EmitVertex();
		gl_Position =  hex[0].bVert;
		texCoord = vec3(-halfSquare,halfSquare,0);
		EmitVertex();
		gl_Position =  hex[0].cVert;
		texCoord = vec3(halfSquare,-halfSquare,0);
		EmitVertex();
		gl_Position =  hex[0].dVert;
		texCoord = vec3(-halfSquare,-halfSquare,0);
		EmitVertex();
		EndPrimitive();
		
}