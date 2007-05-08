// -*- mode: C; -*-
#version 110

/* input:
   gl_Vertex
   gl_Normal		(normal)
   gl_MultiTexCoord0	(texcoord)
*/

varying vec2 texcoord;

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
	vertex_worldspace.z = max(vertex_worldspace.z, 0.0);
	gl_Position = gl_ModelViewProjectionMatrix * vertex_worldspace;

	// compute texture coordinates
	texcoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
}
