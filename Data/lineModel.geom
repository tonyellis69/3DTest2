#version 330

 layout(lines_adjacency) in;
 layout(triangle_strip, max_vertices = 4) out;
 
 uniform vec2 winSize;
 
 out vec2 gsTexCoord;
 
 float thickness = 2.0f;
 

vec2 screenSpace(vec4 vertex) {
	return vec2( vertex.xy / vertex.w ) * winSize;
}



void main() {
	vec2 p0 = screenSpace( gl_in[0].gl_Position );	// start of previous segment
	vec2 p1 = screenSpace( gl_in[1].gl_Position );	// end of previous segment, start of current segment
	vec2 p2 = screenSpace( gl_in[2].gl_Position );	// end of current segment, start of next segment
	vec2 p3 = screenSpace( gl_in[3].gl_Position );	// end of next segment

	//direction of each segment (previous, current, next)
	vec2 v0 = normalize(p1-p0);
	vec2 v1 = normalize(p2-p1);
	vec2 v2 = normalize(p3-p2);

	//normal of each segment (previous, current, next)
	vec2 n0 = vec2(-v0.y, v0.x);
	vec2 n1 = vec2(-v1.y, v1.x);
	vec2 n2 = vec2(-v2.y, v2.x);

	  // generate the triangle strip
	gsTexCoord = vec2(0, 0);
	gl_Position = vec4( (p1 + thickness * n1) / winSize, 0.0, 1.0 );
	EmitVertex();
	gsTexCoord = vec2(0, 1);
	gl_Position = vec4( (p1 - thickness * n1) / winSize, 0.0, 1.0 );
	EmitVertex();
	gsTexCoord = vec2(0, 0);
	gl_Position = vec4( (p2 + thickness * n1) / winSize, 0.0, 1.0 );
	EmitVertex();
	gsTexCoord = vec2(0, 1);
	gl_Position = vec4( (p2 - thickness * n1) / winSize, 0.0, 1.0 );
	EmitVertex();

	EndPrimitive();





/*
	gl_Position = gl_in[1].gl_Position; 
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

	EndPrimitive();
*/
}