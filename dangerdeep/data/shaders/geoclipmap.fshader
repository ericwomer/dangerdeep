// -*- mode: C; -*-

uniform float tex_coord_factor;
/*
	x = range
	y = upper_bound
	z = tex_offset_x
	w = tex_offset_y
 */
uniform vec4 regions[3];
uniform vec2 slope_offset;

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
		weight = max(0.0, (regions[i].x - abs(gl_TexCoord[0].z - regions[i].y)) / regions[i].x);
		col  += weight * texture2D(terrain_texture, (frac(gl_TexCoord[0].xy)*tex_coord_factor)+vec2(regions[i].z, regions[i].w)).xyz;
	}
	//add the weighted texture for slopes
	float slope_weight = max(0, dot(normalize(texture2D(texnormal, texcoordnormal).xyz), vec3(0.0, 0.0, 1.0)));
	col += slope_weight * texture2D(terrain_texture, (frac(gl_TexCoord[0].xy)*tex_coord_factor)+slope_offset).xyz;

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
