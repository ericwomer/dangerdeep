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
uniform vec2 xysize2;
uniform float w_p1;
uniform float w_rcp;
uniform float L_l_rcp;
uniform float N_rcp;
uniform vec2 texcshift;

attribute float z_c;

void main()
{
	vec4 vpos = gl_Vertex;

	vec2 d = abs(vpos.xy - viewpos.xy) * L_l_rcp - xysize2 + vec2(w_p1, w_p1);
	d = clamp(d * w_rcp, 0.0, 1.0);
	alpha = max(d.x, d.y);
	vpos.z = mix(vpos.z, z_c, alpha);

	// shift is 0.5 texel, so it is 1/texres * 0.5
	texcoordnormal = vpos.xy * L_l_rcp * N_rcp + texcshift;
	texcoordnormal_c = vpos.xy * L_l_rcp * 0.5 * N_rcp + texcshift;

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vpos.xyz * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;
	//normal = normalize(gl_Normal);
	//normal = vec3(0.0, 0.0, 1.0);

	// finally compute position
	gl_Position = gl_ModelViewProjectionMatrix * vpos;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
