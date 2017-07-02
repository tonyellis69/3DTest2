#version 330

 layout(points) in;
 layout(line_strip, max_vertices = 16) out;

in boxData {
	vec4 vert;
	vec4 bVert;
	vec4 cVert;
	vec4 dVert;
	
	vec4 eVert;
	vec4 fVert;
	vec4 gVert;
	vec4 hVert;
} box[];


void main() {
		
		gl_Position = box[0].vert;
		EmitVertex();
		gl_Position = box[0].bVert;
		EmitVertex();
		gl_Position = box[0].cVert;
		EmitVertex();
		gl_Position = box[0].dVert;
		EmitVertex();
		gl_Position = box[0].vert;
		EmitVertex();
		
		gl_Position = box[0].eVert;
		EmitVertex();
		gl_Position = box[0].fVert;
		EmitVertex();
		gl_Position = box[0].gVert;
		EmitVertex();
		gl_Position = box[0].hVert;
		EmitVertex();
		gl_Position = box[0].eVert;
		EmitVertex();
		
		EndPrimitive();
		
		gl_Position = box[0].bVert;
		EmitVertex();
		gl_Position = box[0].fVert;
		EmitVertex();
		
		EndPrimitive();
		
		gl_Position = box[0].cVert;
		EmitVertex();
		gl_Position = box[0].gVert;
		EmitVertex();
	
		EndPrimitive();
		
		gl_Position = box[0].dVert;
		EmitVertex();
		gl_Position = box[0].hVert;
		EmitVertex();
		EndPrimitive();
		
}