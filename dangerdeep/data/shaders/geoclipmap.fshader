// -*- mode: C; -*-

varying vec3 lightdir;
//varying vec3 normal;
varying vec2 texcoord;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;
uniform sampler2D texterrain;
uniform sampler2D texnormal;
uniform sampler2D texnormal_c;

void main()
{
	vec3 L = normalize(lightdir);
	// compute normal
	vec3 N = normalize(mix(texture2D(texnormal, texcoordnormal).xyz * 2.0 - 1.0,
			       texture2D(texnormal_c, texcoordnormal_c).xyz * 2.0 - 1.0,
			       alpha));
/*
	vec3 col = mix(texture2D(texnormal, texcoordnormal).xyz,
		       texture2D(texnormal_c, texcoordnormal_c).xyz,
		       alpha);
*/
	vec3 col = texture2D(texterrain, texcoord + N.xy * 0.1).xyz;
	vec3 final_color = col * max(dot(L, N), 0.0);
//	vec3 final_color = texture2D(texnormal, texcoordnormal).xyz;

	// add linear fog
	float fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord * 1.4426940);
	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), 1.0);
}
