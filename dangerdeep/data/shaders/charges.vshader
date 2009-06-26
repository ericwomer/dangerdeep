// long live Vim

/* input:
   gl_Vertex
   gl_Normal		(tangentz)
   gl_MultiTexCoord0	(texcoord)
   tangentx_righthanded	(tangentx,righthanded-factor)
*/


varying vec2 texcoord;
varying vec3 lightdir;
attribute vec4 tangentx_righthanded;

void main()
{

	// compute tangent space
	// gl_Normal = tangentz
	vec3 tangentx = vec3(tangentx_righthanded);
	vec3 tangenty = cross(gl_Normal, tangentx) * tangentx_righthanded.w;

	// compute direction to light in object space (L)
	// light.position.w is 0 or 1, 0 for directional light, 1 for point light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse
			* gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj
			- vec3(gl_Vertex) * gl_LightSource[0].position.w);

	// transform light direction to tangent space
	lightdir.x = dot(tangentx, lightdir_obj);
	lightdir.y = dot(tangenty, lightdir_obj);
	lightdir.z = dot(gl_Normal /*tangentz*/, lightdir_obj);

	// compute texture coordinates
	texcoord = gl_MultiTexCoord0.xy;

	// finally compute position
	gl_Position = ftransform();

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
