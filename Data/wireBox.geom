#version 330

 layout(points) in;
 layout(line_strip, max_vertices = 3) out;

in boxData {
	vec4 vert;
	vec4 opVert;
} box[];


void main() {
		gl_Position = box[0].vert;
		EmitVertex();
		gl_Position = box[0].vert + vec4(box[0].opVert.x,0,0,0);
		EmitVertex();
		gl_Position = box[0].vert + vec4(box[0].opVert.xy,0,0);
		EmitVertex();
		EndPrimitive();
}