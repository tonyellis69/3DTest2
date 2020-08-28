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
	
	float fog;
	
} hex[];

in uint outContent[];

out float outFog;

void main() {

		if (hex[0].fog == 1.0)
			return;

		outFog = hex[0].fog;
		
		gl_Position =  hex[0].vert;	
		EmitVertex();
		gl_Position =  hex[0].bVert;
		EmitVertex();
		EndPrimitive();

		
		gl_Position =  hex[0].bVert;
		EmitVertex();
		gl_Position =  hex[0].cVert;
		EmitVertex();
		EndPrimitive();
		
		
		gl_Position =  hex[0].cVert;
		EmitVertex();
		gl_Position =  hex[0].dVert;
		EmitVertex();
		EndPrimitive();
		
				
		gl_Position =  hex[0].dVert;
		EmitVertex();
		gl_Position =  hex[0].eVert;
		EmitVertex();
		EndPrimitive();
		
		gl_Position =  hex[0].eVert;
		EmitVertex();
		gl_Position =  hex[0].fVert;
		EmitVertex();;
		EndPrimitive();
		
		gl_Position =  hex[0].fVert;
		EmitVertex();
		gl_Position =  hex[0].vert;	
		EmitVertex();
		EndPrimitive();

		
}