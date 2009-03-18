// -*- mode: C; -*-
struct tex_offset {
	float x;
	float y;
};
struct region {
	float range;
	float bound;
	tex_offset tex_off;
};

uniform float tex_coord_factor;
uniform region regions[3];
uniform tex_offset slope_offset;

varying vec3 lightdir;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;

uniform sampler2D terrain_texture;
uniform sampler2D texnormal;
uniform sampler2D texnormal_c;
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
	vec3 N = normalize(mix(texture2D(texnormal, texcoordnormal).xyz * 2.0 - 1.0,
			       texture2D(texnormal_c, texcoordnormal_c).xyz * 2.0 - 1.0,
			       alpha));
	// compute color
	vec3 col = vec3(0.0, 0.0, 0.0);
	float weight;
	// compute simple weight of a texture dependent from it's height and the specified vertical regions
	for(int i=0; i<regions.length(); i++) {
		weight = max(0.0, (regions[i].range - abs(gl_TexCoord[0].z - regions[i].bound)) / regions[i].range);
		col  += weight * texture2D(terrain_texture, (frac(gl_TexCoord[0].xy)*tex_coord_factor)+vec2(regions[i].tex_off.x, regions[i].tex_off.y)).xyz;
	}
	//add the weighted texture for slopes
	col += dot(texture2D(texnormal, texcoordnormal), vec4(0.0, 0.0, 1.0, 0.0)) * texture2D(terrain_texture, (frac(gl_TexCoord[0].xy)*tex_coord_factor)+vec2(slope_offset.x, slope_offset.y)).xyz;

	vec3 final_color = col * (max(dot(L, N), 0.0) * (1.0 - ambient) + ambient);

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
