/* Hoorah for vim */

/* input:
   gl_Vertex
   gl_Normal
*/

varying vec3 lightdir;
varying vec3 viewerdir;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;

uniform vec3 viewpos;
uniform vec3 viewpos_offset;
uniform vec2 xysize2;
uniform float w_p1;
uniform float w_rcp;
uniform float L_l_rcp;
uniform float N_rcp;
uniform vec2 texcshift;
uniform vec2 texcshift2;

#ifdef MIRROR
varying float world_z;
#endif

attribute float z_c;

void main()
{
	vec4 vpos = gl_Vertex;
	
	// the vertices on the level borders need to be blended with the corresponding 
	// values in the next coarsers level. More about this blending function can be
	// found in the geoclipmap papers by Hughues Hoppe.
	vec2 d = abs(vpos.xy - viewpos.xy) * L_l_rcp - xysize2 + vec2(w_p1, w_p1);
	d = clamp(d * w_rcp, 0.0, 1.0);
	alpha = max(d.x, d.y);
	vpos.z = mix(vpos.z, z_c, alpha);

	// Compute the normal map's coord for the fragment shader
	// shift is 0.5 texel, so it is 1/texres * 0.5
	texcoordnormal = vpos.xy * L_l_rcp * N_rcp + texcshift;
	texcoordnormal_c = vpos.xy * L_l_rcp * 0.5 * N_rcp + texcshift2;

	// compute texture coordinates. z is used for height
	gl_TexCoord[0].xy = gl_Vertex.xy;
	gl_TexCoord[0].z = vpos.z;

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - (vpos.xyz + viewpos_offset) * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;

	/* viewerdir obj */
	viewerdir = normalize( vec3(gl_ModelViewMatrixInverse[3])-vec3(gl_Vertex));
	//normal = normalize(gl_Normal);
	//normal = vec3(0.0, 0.0, 1.0);

#ifdef MIRROR
	// clip vertex coordinates at z=0 plane
#ifndef HQSFX
	vpos.z = max(vpos.z, 0.0);
#endif
	world_z = vpos.z;
#endif
	// finally compute position
	gl_Position = gl_ModelViewProjectionMatrix * vpos;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
