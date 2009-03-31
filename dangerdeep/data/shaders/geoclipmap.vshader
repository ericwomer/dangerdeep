// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal
*/

varying vec3 lightdir;
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
	
	vec2 d = abs(vpos.xy - viewpos.xy) * L_l_rcp - xysize2 + vec2(w_p1, w_p1);
	d = clamp(d * w_rcp, 0.0, 1.0);
	alpha = max(d.x, d.y);
	vpos.z = mix(vpos.z, z_c, alpha);
	
	// compute texture coordinates. z is used for height
	gl_TexCoord[0].xy = gl_Vertex.xy;
	gl_TexCoord[0].z = vpos.z;
#if 0
	// simulate earth curvature
	const float earth_radius = 6371000.8;
	float vd = length(vpos.xy - viewpos.xy);
	float r = vd / earth_radius;
	float dz = - r * r;
	vpos.z += dz;
#endif

	// shift is 0.5 texel, so it is 1/texres * 0.5
	texcoordnormal = vpos.xy * L_l_rcp * N_rcp + texcshift;
	texcoordnormal_c = vpos.xy * L_l_rcp * 0.5 * N_rcp + texcshift2;

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - (vpos.xyz + viewpos_offset) * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;
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
