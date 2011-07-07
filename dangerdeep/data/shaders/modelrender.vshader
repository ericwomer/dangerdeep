// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal		(tangentz)
   gl_MultiTexCoord0	(texcoord)
   tangentx_righthanded	(tangentx,righthanded-factor)
*/

#ifdef USE_COLORMAP
varying vec2 texcoord;
#else
varying vec4 color;
#endif
varying vec3 lightdir, halfangle;
#ifdef USE_NORMALMAP
attribute vec4 tangentx_righthanded;
#else
varying vec3 normal;
#endif

#ifdef USE_CAUSTIC
varying vec2 caustic_texcoord;
const vec4 plane_s = vec4(0.05, 0.0, 0.03, 0.0);
const vec4 plane_t = vec4(0.0, 0.05, 0.03, 0.0);

float calculate_caustic_coords(vec3 pos, vec4 plane)
{
	return dot(pos, plane.xyz) + plane.w;
}
#endif

void main()
{
#ifdef USE_CAUSTIC
	caustic_texcoord = vec2( calculate_caustic_coords(gl_Vertex.xyz, plane_s),
   					  		calculate_caustic_coords(gl_Vertex.xyz, plane_t) );
#endif

#ifdef USE_NORMALMAP
	// compute tangent space
	// gl_Normal = tangentz
	vec3 tangentx = vec3(tangentx_righthanded);
	vec3 tangenty = cross(gl_Normal, tangentx) * tangentx_righthanded.w;
#endif

	// compute direction to light in object space (L)
	// light.position.w is 0 or 1, 0 for directional light, 1 for point light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);

	// direction to viewer (E)
	// compute direction to viewer (E) in object space (mvinv*(0,0,0,1) - inputpos)
	vec3 viewerdir_obj = normalize(vec3(gl_ModelViewMatrixInverse[3]) - vec3(gl_Vertex));

	// compute halfangle vector (H = ||L+E||)
	vec3 halfangle_obj = normalize(viewerdir_obj + lightdir_obj);

#ifdef USE_NORMALMAP
	// transform light direction to tangent space
	lightdir.x = dot(tangentx, lightdir_obj);
	lightdir.y = dot(tangenty, lightdir_obj);
	lightdir.z = dot(gl_Normal /*tangentz*/, lightdir_obj);

	halfangle.x = dot(tangentx, halfangle_obj);
	halfangle.y = dot(tangenty, halfangle_obj);
	halfangle.z = dot(gl_Normal /*tangentz*/, halfangle_obj);
#else
	lightdir = lightdir_obj;
	halfangle = halfangle_obj;
	normal = gl_Normal;
#endif

#ifdef USE_COLORMAP
	// compute texture coordinates
	texcoord = gl_MultiTexCoord0.xy;
#else
	color = gl_Color;
#endif

	// finally compute position
	gl_Position = ftransform();

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
