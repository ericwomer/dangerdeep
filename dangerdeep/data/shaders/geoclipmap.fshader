// -*- mode: C; -*-

varying vec3 col;
varying vec3 lightdir;
varying vec3 normal;

void main()
{
	vec3 L = normalize(lightdir);
	vec3 N = normalize(normal);
	vec3 final_color = col * max(dot(L, N), 0.0);
	final_color.z = col.z;

	// add linear fog
	float fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord * 1.4426940);
	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), 1.0);
}
