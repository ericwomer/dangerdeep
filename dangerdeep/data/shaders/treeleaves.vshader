// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal		(tangentz)
   gl_MultiTexCoord0	(texcoord)
   tangentx      	(tangentx)
*/

varying vec2 texcoord;
varying vec3 lightdir, halfangle;
//attribute vec3 tangentx;

uniform vec3 wind_movement;

void main()
{
	// compute tangent space
	// gl_Normal = tangentz
	vec3 tangentx = vec3(1, 0, 0);
	vec3 tangenty = cross(gl_Normal, tangentx);

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
	texcoord = gl_MultiTexCoord0.xy;

	// wind factor
	float wind_factor = 1.0 - gl_MultiTexCoord0.y;
	wind_factor *= wind_factor;
	vec4 pos = gl_Vertex + vec4(wind_movement * wind_factor, 0.0);

	// finally compute position
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * pos;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
