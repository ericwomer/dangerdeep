// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal
*/

varying vec3 col;
varying vec3 lightdir;
//varying vec3 normal;
varying vec2 texcoord;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;

uniform vec3 viewpos;
uniform vec2 xysize2;
uniform float w_p1;
uniform float w_rcp;
uniform float L_l_rcp;
uniform float N_rcp;

attribute float z_c;

void main()
{
	vec4 vpos = gl_Vertex;

	vec2 d = abs(vpos.xy - viewpos.xy) * L_l_rcp;
	d = (d - xysize2 + vec2(w_p1, w_p1));
	d = clamp(d * w_rcp, 0.0, 1.0);
	alpha = max(d.x, d.y);
	vpos.z = mix(vpos.z, z_c, alpha);

	texcoord = vec2(0.0, 1.0 - (vpos.z + 50.0) / 150.0);

	//fixme: so the texture seems to fit, but wtf why?!
	//0.005*256=1.28 so we shift by ca 1.28 texel
	//but why x=0.5 and y=0.75 shift???
	texcoordnormal = vpos.xy * L_l_rcp * N_rcp /* + vec2(+0.005, +0.005)*/;
	texcoordnormal_c = vpos.xy * L_l_rcp * 0.5 * N_rcp /*+ vec2(+0.005, +0.005)*/;
//	texcoordnormal = vpos.xy * L_l_rcp * N_rcp + vec2(0.5-0.005, 0.75-0.005);
//	texcoordnormal_c = vpos.xy * L_l_rcp * 0.5 * N_rcp + vec2(0.5-0.005, 0.75-0.005);
	// maybe the texture is y-flipped?
	//texcoordnormal.y = -texcoordnormal.y;
	//texcoordnormal_c.y = -texcoordnormal_c.y;

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vpos.xyz * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;
	//normal = normalize(gl_Normal);
	//normal = vec3(0.0, 0.0, 1.0);

	col = gl_Color.xyz;
	//col.z = d.x;
	//col.y = d.y;
	//col.y = 1.0 - (vpos.z + 0.0)/100.0;

	// finally compute position
	gl_Position = gl_ModelViewProjectionMatrix * vpos;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
