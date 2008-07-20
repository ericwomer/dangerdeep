// -*- mode: C; -*-

varying vec3 lightdir;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;
uniform sampler2D texcolor;
uniform sampler2D texcolor_c;
uniform sampler2D texnormal;
uniform sampler2D texnormal_c;
#ifdef MIRROR
varying float world_z;
#endif

const float ambient = 0.3;

void main()
{
#ifdef MIRROR
#ifdef DO_REAL_CLIPPING
	if (world_z < 0.0)
		discard;
	float alpha = 1.0;
#else
	float alpha = world_z <= 0.0 ? 0.0 : 1.0; // does this work without if? fixme check asm output
#endif
#endif
	vec3 L = normalize(lightdir);
	// compute normal
	vec3 N = normalize(mix(texture2D(texnormal, texcoordnormal).xyz * 2.0 - 1.0,
			       texture2D(texnormal_c, texcoordnormal_c).xyz * 2.0 - 1.0,
			       alpha));
	// compute color
	vec3 col = mix(texture2D(texcolor, texcoordnormal).xyz,
		       texture2D(texcolor_c, texcoordnormal_c).xyz,
		       alpha);
	vec3 final_color = col * (max(dot(L, N), 0.0) * (1.0 - ambient) + ambient);
//	final_color = mix(final_color, vec3(1.0, 1.0, 1.0), alpha);
//	final_color.z = alpha;
//	vec3 final_color = texture2D(texnormal, texcoordnormal).xyz;

	// add linear fog
	float fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord * 1.4426940);
	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor),
#ifdef MIRROR
			    alpha
#else
			    1.0
#endif
			    );
}
