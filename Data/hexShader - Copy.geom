#version 330

 layout(points) in;
 layout(line_strip, max_vertices = 12) out;
 
in squareData {
	vec4 vert;
	vec4 bVert;
	vec4 cVert;
	vec4 dVert;
	vec4 eVert;
	vec4 fVert;
	vec4 origin;
	
	vec2 lineCentre[6];
	
} hex[];


out vec2 lineCentre;

void main() {
		
		gl_Position =  hex[0].vert;
		lineCentre = hex[0].lineCentre[0];	
		EmitVertex();
		gl_Position =  hex[0].bVert;
		lineCentre = hex[0].lineCentre[1];	
		EmitVertex();
		EndPrimitive();

		
		gl_Position =  hex[0].bVert;
		lineCentre = hex[0].lineCentre[1];	
		EmitVertex();
		gl_Position =  hex[0].cVert;
		lineCentre = hex[0].lineCentre[2];	
		EmitVertex();
		EndPrimitive();
		
		
		gl_Position =  hex[0].cVert;
		lineCentre = hex[0].lineCentre[2];	
		EmitVertex();
		gl_Position =  hex[0].dVert;
		lineCentre = hex[0].lineCentre[3];	
		EmitVertex();
		EndPrimitive();
		
				
		gl_Position =  hex[0].dVert;
		lineCentre = hex[0].lineCentre[3];	
		EmitVertex();
		gl_Position =  hex[0].eVert;
		lineCentre = hex[0].lineCentre[4];	
		EmitVertex();
		EndPrimitive();
		
		gl_Position =  hex[0].eVert;
		lineCentre = hex[0].lineCentre[4];	
		EmitVertex();
		gl_Position =  hex[0].fVert;
		lineCentre = hex[0].lineCentre[5];	
		EmitVertex();;
		EndPrimitive();
		
		gl_Position =  hex[0].fVert;
		lineCentre = hex[0].lineCentre[5];	
		EmitVertex();
		gl_Position =  hex[0].vert;
		lineCentre = hex[0].lineCentre[0];	
		EmitVertex();
		EndPrimitive();

		
}