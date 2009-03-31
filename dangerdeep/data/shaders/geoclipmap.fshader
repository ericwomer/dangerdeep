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
uniform float tex_stretch_factor;

uniform sampler2D terrain_texture0;
uniform sampler2D terrain_texture1;
uniform sampler2D terrain_texture2;
uniform sampler2D bump_texture;
uniform sampler2D slope_texture;
#ifdef MIRROR
varying float world_z;
#endif

const float ambient = 0.3;

float compute_weight(float range, float bound, float height) {
	return max(0.0, (range - abs(height - bound)) / range);
}

vec3 get_color(sampler2D texture, vec2 coord) {
	return mix(
			   texture2D(texture, coord*tex_stretch_factor).xyz, 
			   texture2D(texture, coord*(tex_stretch_factor/2.0)).xyz, 
			   alpha
			   );
}
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
	vec2 coord = gl_TexCoord[0].xy;

	// compute simple weight of a texture dependent from it's height and the specified vertical regions
	weight = compute_weight(regions[0].x, regions[0].y, height);
	blended_color = mix(blended_color, get_color(terrain_texture0, coord), weight);

	weight = compute_weight(regions[1].x, regions[1].y, height);
	blended_color = mix(blended_color, get_color(terrain_texture1, coord), weight);
	
	weight = compute_weight(regions[2].x, regions[2].y, height);
	blended_color = mix(blended_color, get_color(terrain_texture2, coord), weight);

	float slope = 1.0 - dot(N, vec3(0.0, 0.0, 1.0));
	vec3 slope_color = get_color(slope_texture, coord).xyz;
	blended_color = mix(blended_color, slope_color, slope);
	
	vec3 final_color = vec3(blended_color * (max(dot(L, N), 0.0) * (1.0 - ambient) + ambient));
	gl_FragColor = vec4(final_color,1.0);
	/*
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
	 */
}
