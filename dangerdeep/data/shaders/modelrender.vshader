// -*- mode: C; -*-
#version 110

/* input:
   gl_Vertex
   gl_Normal		(tangentz)
   gl_MultiTexCoord0	(texcoord)
   tangentx_righthanded	(tangentx,righthanded-factor)
*/

/* can we give names to default attributes? or do we need to use named attributes for that?
   how to give named attributes with vertex buffer objects?
   we could assign them to a variable, but would that be efficient? an unnecessary copy.
   but the shader compiler should be able to optimize that...
   the way should be to use vertex attributes (together with vertex arrays or VBOs),
   that have a name and use that name here.
   until then, access sources directly or via a special variable.
*/

varying vec2 texcoord;
varying vec3 lightdir, halfangle;
attribute vec4 tangentx_righthanded;

void main()
{
	// compute tangent space
	// gl_Normal = tangentz
	const vec3 tangentx = vec3(tangentx_righthanded);
	const vec3 tangenty = cross(gl_Normal, tangentx) * tangentx_righthanded.w;

	// compute direction to light in object space (L)
	// light.position.w is 0 or 1, 0 for directional light, 1 for point light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);

	// direction to viewer (E)
	// compute direction to viewer (E) in object space (mvinv*(0,0,0,1) - inputpos)
	vec3 viewerdir_obj = normalize(vec3(gl_ModelViewMatrixInverse[3]) - vec3(gl_Vertex));

	// compute halfangle vector (H = ||L+E||)
	vec3 halfangle_obj = normalize(viewerdir_obj + lightdir_obj);

	// transform light direction to tangent space
	lightdir.x = dot(tangentx, lightdir_obj);
	lightdir.y = dot(tangenty, lightdir_obj);
	lightdir.z = dot(gl_Normal /*tangentz*/, lightdir_obj);

	halfangle.x = dot(tangentx, halfangle_obj);
	halfangle.y = dot(tangenty, halfangle_obj);
	halfangle.z = dot(gl_Normal /*tangentz*/, halfangle_obj);

	// compute texture coordinates
	texcoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;

	// finally compute position
	gl_Position = ftransform();

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
