/* Vim rocks, long live vim */

varying vec3 lightdir;
varying vec3 viewerdir;
varying vec2 texcoordnormal;
varying vec2 texcoordnormal_c;
varying float alpha;

/* following 3 variables should be in a texmap, specular intensity in R,
   specular roughness in G, Buratti weight factor in B */
float ks; /* = foobar; <<< specular intensity/specularity,R channe*/
/* roughness [0,1] 0 = infinitely sharp specular, 1 = spread specular,
   ideal for smooth surfaces .1, .2, for rough surfaces, .3, .4.
   Should be in G channel */
float roughness = 0.3;
/* Buratti weight factor [0,1] k=0 = Lambertian diffuse, k=1 = Lommel-Seeliger
   with a single scattering effect (gives idea of rough surface). For
   smooth surfaces, .1, .2, for rough surfaces, .3, .4, for vegetation, .7,.8.
   Should be in B channel */
float k = 0.0; /* Buratti weight */

const float ambient = 0.3;

uniform int above_water;
uniform vec3 viewpos;

uniform sampler2D texnormal;
uniform sampler2D texnormal_c;

struct region {
	float max;
	float min;
};

region reg_sand = region(float(0), float(-9000));
region reg_mud = region(float(100), float(1));
region reg_grass = region(float(2000), float(101));
region reg_rock = region(float(4000), float(2001));
region reg_snow = region(float(9000), float(4001));

uniform float tex_stretch_factor;

uniform sampler2D base_texture;
uniform sampler2D noise_texture;

uniform sampler2D sand_texture;
uniform sampler2D mud_texture;
uniform sampler2D forest_texture;
uniform sampler2D grass_texture;
uniform sampler2D rock_texture;
uniform sampler2D snow_texture;

uniform sampler2D forest_brdf_texture;
uniform sampler2D rock_brdf_texture;

#ifdef MIRROR
varying float world_z;
#endif

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
	
	vec3 Ln = normalize(lightdir);
	// compute normal
	vec3 Nn = normalize(mix(texture2D(texnormal, texcoordnormal).xyz
				* 2.0 - 1.0, texture2D(texnormal_c, texcoordnormal_c).xyz
				* 2.0 - 1.0, alpha));
	// viewerdir
	vec3 In = -normalize(viewerdir);

	// preset quantities
	float costheta = max(0.0, dot(In, Nn));
	float cospsi = max( 0.0, dot(Ln, Nn) );
	// build reflection vector for Schlick's (isotropic) specular term
	vec3 Rn = reflect( Ln, Nn );
	float cosphi = max( 0.0, dot(Rn, In) );
	
	// compute color
	vec3 terrain_color;
	vec2 coord = gl_TexCoord[0].xy;

	float noise = 0.5+get_color(noise_texture, coord).r;
	float height = gl_TexCoord[0].z * noise;
	float height_weight = smoothstep(5500, 4500, viewpos.z-height);

	if(height_weight > 0.0) {
		//blend between base texture and detail texture
		float weight;
		float slope = (1.0 - dot(Nn, vec3(0.0, 0.0, 1.0)));
		if(height <= reg_sand.max) {
			weight = clamp(10.0/(reg_mud.min-height), 0.0, 1.0);
			terrain_color = mix(get_color(sand_texture, coord),	get_color(mud_texture, coord), weight);
		}	else if(height <= reg_mud.max) {
			weight = clamp(10.0/(reg_grass.min-height), 0.0, 1.0);
			terrain_color = mix(get_color(mud_texture, coord), get_color(forest_texture, coord), weight);
		}	else if(height <= reg_grass.max) {
			weight = clamp(10.0/(reg_rock.min-height), 0.0, 1.0);
			terrain_color = mix(get_color(forest_texture, coord), get_color(rock_texture, coord), weight);
		}	else if(height <= reg_rock.max) {
			weight = clamp(10.0/(reg_snow.min-height), 0.0, 1.0);
			terrain_color = mix(get_color(rock_texture, coord),	get_color(snow_texture, coord), weight);
		}else if(height <= reg_snow.max) {
			terrain_color = get_color(snow_texture, coord);
		}
		//blend in rock texture on steep slopes
		terrain_color = mix(terrain_color, get_color(rock_texture, coord), slope);

		//blend with the base texture
		terrain_color = mix(texture2D(base_texture, vec2(clamp((height+100.0),0,3100.0)/3100.0,1)).rgb, terrain_color, height_weight);
} else {
	//use only the base texture
	terrain_color = texture2D(base_texture, vec2(clamp((height+100.0),0,3100.0)/3100.0,1)).rgb;
}

/*	vec3 brdf = mix(get_color(forest_brdf_texture, coord), get_color(rock_brdf_texture, coord), slope).rgb;

ks = brdf.r;
roughness += brdf.g;
k = brdf.b;
*/

	//Bump mapping
#ifdef HQSFX	
	//N = normalize(N+get_color(bump_texture, coord)*10.0);
#endif

	/* Build Buratti diffuse term and Schlick specular term */
#ifdef HQSFX
	float diffterm = k * (cospsi / (cospsi+costheta)) + (1.0-k) * cospsi;
	float nr = 1.0 / (roughness * roughness);
	float coeff = cosphi / (nr - nr * cosphi + cosphi);
	/* light color = gl_LightSource[x].diffuse */
	vec3 final_color = vec3(gl_LightSource[0].diffuse) * (terrain_color * diffterm + coeff * ks );
#else
	vec3 final_color = vec3(terrain_color * cospsi );
#endif

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
