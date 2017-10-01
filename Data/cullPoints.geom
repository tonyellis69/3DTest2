#version 330

 layout(lines) in;
 layout(points, max_vertices = 1) out;

in TPointStatus {
	vec3 point; //The point we're investigating.
	float cull;  //1= cull, 0=keep;
} pointStatus[];

out vec3 gl_Position;

void main() {
		
		if (pointStatus[0].cull > 0.5f) 
		{
			gl_Position = pointStatus[0].point;
			EmitVertex();
			
		}
		EndPrimitive();
		
		
		
}