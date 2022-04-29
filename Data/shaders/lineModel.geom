#version 330

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 7) out;
//layout(line_strip, max_vertices = 8) out; //uncomment for wireframe
  
 uniform vec2 winSize; //multiplay by to get screenspace, divide by to get normspace
 
 out vec2 gsTexCoord;
 
 in vColour {
	vec4 colour;
} colour_in[];

out vec4 gsColour;

 
 float thickness =  2.0f; //2.0f;
 float miter_limit = 0.75f; //1.0f; //1 = always, -1 = never. 0.75f good default.
 //NB 1 (always) creates glitches with very small lines at sharpish angles, which -1 and 0.75f seem to fix

//Put vertex into screen space, eg, 1280 x 1060
vec2 screenSpace(vec4 vertex) {
	return vec2( vertex.xy / vertex.w ) * winSize;
}


void main() {
	vec2 p0 = screenSpace( gl_in[0].gl_Position );	// start of previous segment
	vec2 p1 = screenSpace( gl_in[1].gl_Position );	// end of previous segment, start of current segment
	vec2 p2 = screenSpace( gl_in[2].gl_Position );	// end of current segment, start of next segment
	vec2 p3 = screenSpace( gl_in[3].gl_Position );	// end of next segment
	
	//naive culling
	vec2 area = winSize * 1.2f;
	if( p1.x < -area.x || p1.x > area.x ) return;
	if( p1.y < -area.y || p1.y > area.y ) return;
	if( p2.x < -area.x || p2.x > area.x ) return;
	if( p2.y < -area.y || p2.y > area.y ) return;
	
	

	//direction of each segment (previous, current, next)
	vec2 v0 = normalize(p1-p0);
	vec2 v1 = normalize(p2-p1);
	vec2 v2 = normalize(p3-p2);

	//normal of each segment (previous, current, next)
	vec2 n0 = vec2(-v0.y, v0.x);
	vec2 n1 = vec2(-v1.y, v1.x);
	vec2 n2 = vec2(-v2.y, v2.x);
	
	//average normals at either end of segment to find miter
	vec2 miter_a = normalize(n0 + n1);	
	vec2 miter_b = normalize(n1 + n2);	
	
	//for (pass = 0; pass < 2; pass += 1.0f) {
		
	//	if (pass == 1.0f)
	//		thickness = 6.0f;

	//find length of miter by projecting it onto normal for this segment
	float length_a = thickness / dot(miter_a, n1);
	float length_b = thickness / dot(miter_b, n1);
	
	
	gsColour = colour_in[0].colour;
	

	  // prevent excessively long miters at sharp corners
  if( dot(v0,v1) < -miter_limit ) {
	miter_a = n1;
	length_a = thickness;
	
	// close the gap
	if( dot(v0,n1) > 0 ) { // /- join
		gsTexCoord = vec2(0, 0);
		gl_Position = vec4( (p1 + thickness * n0) / winSize, 0.0, 1.0 );
		EmitVertex();
		gsTexCoord = vec2(0, 0);
		gl_Position = vec4( (p1 + thickness * n1) / winSize, 0.0, 1.0 );
		EmitVertex();
		gsTexCoord = vec2(0, 0.5);
		gl_Position = vec4( p1 / winSize, 0.0, 1.0 );
		EmitVertex();
		EndPrimitive();
	}
	else { // \_ join
		gsTexCoord = vec2(0, 1);
		gl_Position = vec4( (p1 - thickness * n1) / winSize, 0.0, 1.0 );
		EmitVertex();		
		gsTexCoord = vec2(0, 1);
		gl_Position = vec4( (p1 - thickness * n0) / winSize, 0.0, 1.0 );
		EmitVertex();
		gsTexCoord = vec2(0, 0.5);
		gl_Position = vec4( p1 / winSize, 0.0, 1.0 );
		EmitVertex();
		EndPrimitive();
	}
  }
  
	
	if( dot(v1,v2) < -miter_limit ) {
		miter_b = n1;
		length_b = thickness;
	}
	

	  // generate the triangle strip
	gsTexCoord = vec2(0, 0);
	gl_Position = vec4( (p1 + length_a * miter_a) / winSize, 0.0, 1.0 );
	EmitVertex();
	gsTexCoord = vec2(0, 1);
	gl_Position = vec4( (p1 - length_a * miter_a) / winSize, 0.0, 1.0 );
	EmitVertex();
	
	
	gsTexCoord = vec2(0, 0);
	gl_Position = vec4( (p2 + length_b * miter_b) / winSize, 0.0, 1.0 );
	EmitVertex();
	
	gsTexCoord = vec2(0, 1);
	gl_Position = vec4( (p2 - length_b * miter_b) / winSize, 0.0, 1.0 );
	EmitVertex();
	
	
	
	//uncomment for wireframe
	/*
	gsTexCoord = vec2(0, 1);
	gl_Position = vec4( (p2 - length_b * miter_b) / winSize, 0.0, 1.0 );
	EmitVertex();
	
	gsTexCoord = vec2(0, 0);
	gl_Position = vec4( (p2 + length_b * miter_b) / winSize, 0.0, 1.0 );
	EmitVertex();
	
	gsTexCoord = vec2(0, 0);
	gl_Position = vec4( (p1 + length_a * miter_a) / winSize, 0.0, 1.0 );
	EmitVertex(); 
	*/

	EndPrimitive();

	//}

}