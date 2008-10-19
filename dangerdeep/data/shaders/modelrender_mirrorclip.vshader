// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal		(normal)
   gl_MultiTexCoord0	(texcoord)
*/

varying vec2 texcoord;
varying float world_z;

void main()
{
	// compute direction to light in object space (L)
	// light.position.w is 0 or 1, 0 for directional light, 1 for point light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);

	// diffuse lighting per vertex
	gl_FrontColor = gl_LightSource[0].diffuse * max(dot(lightdir_obj, gl_Normal), 0.0);

	// transform vertex to projection space (clip coordinates)
	// transform to clip space (only transform x/y/z as w is one and clip3 is 0,0,0,1)
	vec4 vertex_worldspace = (gl_TextureMatrix[1] * gl_Vertex);
	/*
	  how to get rid of the texture matrix usage:
	  give 4 coefficients of a plane equation as uniform.
	  geometry is clipped against that plane.
	  modelview matrix is used as usual.
	  clipplane equation is computed from transformation of object to world.

	vec4 clipplane = vec4(0.0, 0.0, 1.0, 0.0); // <- give as uniform!
	float dist = dot(gl_Vertex.xyz, clipplane.xyz);
	vec3 vpos_in_plane = gl_Vertex.xyz - clipplane.xyz * dist;
	float clip_d = dist + clipplane.w;
	clip_d = max(0.0, clip_d);
	vec4 vpos = vec4(vpos_in_plane + clipplane.xyz * (clip_d - clipplane.w), gl_Vertex.w);
	gl_Position = gl_ModelViewProjectionMatrix * vpos;
	*/
#ifndef HQSFX
	vertex_worldspace.z = max(vertex_worldspace.z, 0.0);
#endif
	world_z = vertex_worldspace.z;
	gl_Position = gl_ModelViewProjectionMatrix * vertex_worldspace;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;

	// compute texture coordinates
	texcoord = gl_MultiTexCoord0.xy;
}
