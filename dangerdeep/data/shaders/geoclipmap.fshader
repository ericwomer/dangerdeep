// -*- mode: C; -*-

varying vec3 lightdir;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;

uniform sampler2D texnormal;
uniform sampler2D texnormal_c;

/*
	x = range
	y = upper_bound
 */
uniform vec2 regions[3];

uniform sampler2D terrain_texture0;
uniform sampler2D terrain_texture1;
uniform sampler2D terrain_texture2;
uniform sampler2D bump_texture;
uniform sampler2D slope_texture;
#ifdef MIRROR
varying float world_z;
#endif

const float ambient = 0.3;

void main()
{
#ifdef MIRROR
#ifdef HQSFX
	if (world_z < 0.0)
		discard;
#endif
#endif
	vec3 L = normalize(lightdir);
	// compute normal
	vec3 N = normalize(mix(texture2D(texnormal,     texcoordnormal).xyz * 2.0 - 1.0,
			   			   texture2D(texnormal_c, texcoordnormal_c).xyz * 2.0 - 1.0,
			   			   alpha));
	// compute color
	float weight;
	float height = gl_TexCoord[0].z;
	vec3 blended_color = vec3(0.0, 0.0, 0.0);
	vec2 coord = frac(gl_TexCoord[0].xy);

	// compute simple weight of a texture dependent from it's height and the specified vertical regions
	weight = max(0.0, (regions[0].x - abs(height - regions[0].y)) / regions[0].x);
	blended_color = mix(blended_color, texture2D(terrain_texture0, coord).xyz, weight);

	weight = max(0.0, (regions[1].x - abs(height - regions[1].y)) / regions[1].x);
	blended_color = mix(blended_color, texture2D(terrain_texture1, coord).xyz, weight);
	
	weight = max(0.0, (regions[2].x - abs(height - regions[2].y)) / regions[2].x);
	blended_color = mix(blended_color, texture2D(terrain_texture2, coord).xyz, weight);

	float slope = 1.0 - dot(N, vec3(0.0, 0.0, 1.0));
	vec3 slope_color = texture2D(slope_texture, coord).xyz;
	blended_color = mix(blended_color, slope_color, slope);
	
	vec3 final_color = vec3(blended_color * (max(dot(L, N), 0.0) * (1.0 - ambient) + ambient));

	// add linear fog
	float fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord * 1.4426940);
	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor),

#ifdef MIRROR
#ifdef HQSFX
			    1.0
#else
			    world_z <= 0.0 ? 0.0 : 1.0
#endif
#else
			    1.0
#endif
			    );
}
