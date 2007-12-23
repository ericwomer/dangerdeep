// -*- mode: C; -*-

uniform sampler2D tex_cloud;

varying vec2 texcoord;
varying vec3 lightdir;
varying vec3 lightcolor;

void main()
{
	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

	vec4 t = texture2D(tex_cloud, texcoord.xy);
	vec3 N = normalize(vec3(t.x, t.y, -t.z) * 2.0 - 1.0);

	float c = clamp(dot(N, L), 0.0, 1.0);
	c = 0.5 + 0.5 * c;
	const vec3 cloudcol = vec3(1.0, 1.0, 1.0);
	vec3 final_color = mix(vec3(c, c, c), cloudcol * lightcolor, t.w);

	// add linear fog
//	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	// output color is a mix between fog and final color
//	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), t.w);
	gl_FragColor = vec4(final_color, t.w);
}
