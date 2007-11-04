// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal
*/

varying vec3 col;
varying vec3 lightdir;
varying vec3 normal;

uniform vec3 viewpos;
uniform vec2 xysize2;
uniform float w;

attribute float z_c;

void main()
{
	vec4 vpos = gl_Vertex;

	vec2 d = vpos.xy - viewpos.xy;
	d.x = abs(d.x);
	d.y = abs(d.y);
	d = (d - xysize2 + vec2(w + 1.0, w + 1.0));
	d.x = min(max(d.x/w, 0.0), 1.0);
	d.y = min(max(d.y/w, 0.0), 1.0);
	d.x = max(d.x, d.y);
	vpos.z = mix(vpos.z, z_c, d.x);

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vpos.xyz * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;
	normal = normalize(gl_Normal);

	col = gl_Color.xyz;
	col.z = d.x;

	// finally compute position
	gl_Position = gl_ModelViewProjectionMatrix * vpos;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
