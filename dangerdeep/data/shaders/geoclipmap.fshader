// -*- mode: C; -*-

varying vec3 lightdir;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;

uniform int above_water;

uniform sampler2D texnormal;
uniform sampler2D texnormal_c;

struct region {
	float max;
	float min;
};

region reg_sand = region(0, -9000);
region reg_mud = region(40, 1);
region reg_grass = region(2000, 41);
region reg_rock = region(4000, 2001);
region reg_snow = region(9000, 4001);

uniform float tex_stretch_factor;

uniform sampler2D sand_texture;
uniform sampler2D mud_texture;
uniform sampler2D forest_texture;
uniform sampler2D noise_texture;
uniform sampler2D rock_texture;
uniform sampler2D snow_texture;

#ifdef MIRROR
varying float world_z;
#endif

const float ambient = 0.3;

float compute_weight(float range, float bound, float height) {
	return max(0.0, (range - abs(height - bound)) / range);
}

vec3 get_color(sampler2D texture, vec2 coord) {
	
	return mix(
			   texture2D(texture, (coord*tex_stretch_factor)).xyz, 
			   texture2D(texture, (coord*(tex_stretch_factor/2.0))).xyz, 
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
	
	//clipping
	if((above_water > 0 && gl_TexCoord[0].z < 0.0) ||
	   (above_water < 0 && gl_TexCoord[0].z > 0.0))
		discard;
	
	vec3 L = normalize(lightdir);
	// compute normal
	vec3 N = normalize(mix(texture2D(texnormal, texcoordnormal).xyz * 2.0 - 1.0,
			   			   texture2D(texnormal_c, texcoordnormal_c).xyz * 2.0 - 1.0,
			   			   alpha));
	
	// compute color
	vec3 terrain_color;
	vec2 coord = gl_TexCoord[0].xy;

	float weight;
	float noise = abs(get_color(noise_texture, coord).x);
	float height = gl_TexCoord[0].z;
	float slope = (1.0 - dot(N, vec3(0.0, 0.0, 1.0)));
	
//	if(slope >= 0.4) terrain_color = get_color(rock_texture, coord);

	height *= noise;

	if(height <= reg_sand.max) {
		weight = clamp(10.0/(reg_mud.min-height), 0.0, 1.0);
		terrain_color = mix(get_color(sand_texture, coord), get_color(mud_texture, coord), weight);
	}	else if(height <= reg_mud.max) {
		weight = clamp(10.0/(reg_grass.min-height), 0.0, 1.0);
		terrain_color = mix(get_color(mud_texture, coord), get_color(forest_texture, coord), weight);
	}	else if(height <= reg_grass.max) {
		weight = clamp(10.0/(reg_rock.min-height), 0.0, 1.0);
		terrain_color = mix(get_color(forest_texture, coord), get_color(snow_texture, coord), weight);
	}	else if(height <= reg_rock.max) {
		weight = clamp(10.0/(reg_snow.min-height), 0.0, 1.0);
		terrain_color = mix(get_color(rock_texture, coord), get_color(snow_texture, coord), weight);
	}else if(height <= reg_snow.max) {
		terrain_color = get_color(snow_texture, coord);
	}

	terrain_color = mix(terrain_color, get_color(rock_texture, coord), slope);



	//Bump mapping
	#ifdef HQSFX	
	//N = normalize(N+get_color(bump_texture, coord)*10.0);
	#endif
	
	vec3 final_color = vec3(terrain_color * max( 0.0, dot(L, N)));
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
