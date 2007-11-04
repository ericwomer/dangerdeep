// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal
*/

varying vec3 col;
varying vec3 lightdir;
varying vec3 normal;
varying vec2 texcoord;

uniform vec3 viewpos;
uniform vec2 xysize2;
uniform float w;
uniform float L_l;

attribute float z_c;

void main()
{
	vec4 vpos = gl_Vertex;

	float wl = w * L_l;
	vec2 d = vpos.xy - viewpos.xy;
	d.x = abs(d.x);
	d.y = abs(d.y);
	d = (d - xysize2 + vec2(wl + L_l, wl + L_l));
	d.x = min(max(d.x/wl, 0.0), 1.0);
	d.y = min(max(d.y/wl, 0.0), 1.0);
	d.x = max(d.x, d.y);
	vpos.z = mix(vpos.z, z_c, d.x);

	texcoord = vec2(0.0, 1.0 - (vpos.z + 50.0) / 150.0);

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vpos.xyz * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;
	normal = normalize(gl_Normal);
	//normal = vec3(0.0, 0.0, 1.0);

	col = gl_Color.xyz;
	col.z = d.x;

	// finally compute position
	gl_Position = gl_ModelViewProjectionMatrix * vpos;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
