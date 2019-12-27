// -*- mode: C; -*-

uniform sampler2D tex_diff;
uniform sampler2D tex_nrml;
uniform vec3 light_dir;

varying vec2 texcoord;

void main()
{
	vec4 c = texture2D(tex_diff, texcoord.xy);
	vec3 n = texture2D(tex_nrml, texcoord.xy).xyz;
	float br = dot(normalize(n), light_dir);
	//br = 0.5 + br * 0.5;
	br *= 2.0;
	gl_FragColor = c * br;
}
