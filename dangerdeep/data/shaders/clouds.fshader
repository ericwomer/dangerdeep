// -*- mode: C; -*-

uniform sampler2D tex_cloud;

varying vec2 texcoord;
varying vec3 lightdir;
varying vec3 lightcolor;
varying float horizon_alpha;

const float density = 0.25; // 0.0 = full density
const float density_rcp = 1.33333; // 1/(1-density)

void main()
{
	// get and normalize vector to light source
//	vec3 L = normalize(lightdir);
	vec3 L = normalize(vec3(lightdir.x, lightdir.y, -0.1));

	const float d = 1.0/256.0;
	const float zh = -0.5;
	float hr = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(d, 0.0)).x;
	float hu = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(0.0, d)).x;
	float hl = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(-d, 0.0)).x;
	float hd = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(0.0, -d)).x;
	float hr2 = texture2D(tex_cloud, texcoord.xy*4.0 + vec2(d, 0.0)).x;
	float hu2 = texture2D(tex_cloud, texcoord.xy*4.0 + vec2(0.0, d)).x;
	float hl2 = texture2D(tex_cloud, texcoord.xy*4.0 + vec2(-d, 0.0)).x;
	float hd2 = texture2D(tex_cloud, texcoord.xy*4.0 + vec2(0.0, -d)).x;
	vec3 N = normalize(vec3(hl - hr + (hl2 - hr2)*0.25, hd - hu + (hd2 - hu2)*0.25, zh));// * 0.5 + 0.5;
	float alpha = (hr - density) * density_rcp * horizon_alpha;

	float c = clamp(dot(N, L), 0.0, 1.0);
	const vec3 cloudcol = vec3(0.85, 0.85, 0.9);
	float mixfac = 0.5;
	vec3 final_color = mix(cloudcol * lightcolor, vec3(c, c, c), mixfac);

	// add linear fog
//	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	// output color is a mix between fog and final color
//	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), t.w);
	gl_FragColor = vec4(final_color, alpha);
}
