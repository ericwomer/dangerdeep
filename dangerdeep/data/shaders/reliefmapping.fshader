// -*- mode: C; -*-

#define USE_RELIEF_MAPPING
#define USE_SHADOWS

uniform sampler2D tex_color;	// (diffuse) color map, RGB
uniform sampler2D tex_normal;	// normal map, RGBA

varying vec2 texcoord;
varying vec3 lightdir, halfangle;
varying vec3 viewerpos;
varying float zdist;

const float depth_factor = 0.015;

void main()
{
#ifdef USE_RELIEF_MAPPING
	float dist_factor = max(1.0-zdist*0.2,0.0);
	vec2 texcoord_new = texcoord.xy;
	if (dist_factor > 0.0) {
		// compute new texcoord
		vec3 V = normalize(viewerpos);
		vec2 s = -V.xy * depth_factor / V.z;

		const int linear_search_steps = 20;
		float size = 1.0 / float(linear_search_steps);
		float depth = 0.0;
		float best_depth = 1.0;

		for(int i = 0; i < linear_search_steps - 1; ++i) {
			depth += size;
			float t = texture2D(tex_normal, texcoord_new + s * depth).a;
			// fixme: is "break" here faster?
			if(best_depth > 0.996)
				if(depth >= t)
					best_depth = depth;
		}
		depth = best_depth;

		const int binary_search_steps = 5;
		for(int i = 0; i < binary_search_steps; ++i) {
			size *= 0.5;
			float t = texture2D(tex_normal, texcoord_new + s * depth).a;
			if(depth >= t) {
				best_depth = depth;
				depth -= 2.0 * size;
			}
			depth += size;
		}

		texcoord_new += s * best_depth;
	}
#else
	vec2 texcoord_new = texcoord.xy;
#endif

	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

	// get and normalize normal vector from texmap
	vec3 N = normalize(vec3(texture2D(tex_normal, texcoord_new)) * 2.0 - 1.0);

#if 0
	// compute specular color
	// get and normalize half angle vector
	vec3 H = normalize(halfangle);

	// compute resulting specular color
	vec3 specular_color = vec3(gl_FrontMaterial.specular) *
		pow(max(dot(H, N), 0.0), gl_FrontMaterial.shininess);
#else
	vec3 specular_color = vec3(0.0, 0.0, 0.0);
#endif

	// compute diffuse color
	vec3 diffuse_color = vec3(texture2D(tex_color, texcoord_new));

	// handle ambient
	float NdotL = max(dot(L, N), 0.0);
#ifdef USE_SHADOWS
	if (NdotL > 0.0) {
		// shadows
		const int shadow_steps = 5; //int(mix(20.0, 5.0, L.z)); // not too many steps!
		//fixme: need to invert heights - how strange!
		//do we need to invert them above too?!
		float h = 1.0-texture2D(tex_normal, texcoord_new).a; // fixme reuse value from above!
		// use shadow_steps loop count to get to height=1.0
		float step = (1.0 - h) / float(shadow_steps);
		vec2 d = L.xy * depth_factor * step / L.z;
		h += step * 0.1;
		vec2 texc = texcoord_new;
		float shadow_mult = 1.0;
		float maxh = 0.0;
		for (int i = 0; i < shadow_steps; ++i) {
			h += step;
			texc += d;
			float th = 1.0-texture2D(tex_normal, texc).a;
			if (th >= h) {
				maxh = max(maxh, th - h);
			}
		}
		shadow_mult = 1.0 - clamp(maxh * 2.0, 0.0, 1.0);
		NdotL *= shadow_mult;
	}
#endif
	diffuse_color = diffuse_color * mix(NdotL, 1.0, gl_LightSource[0].ambient.r);

	// final color of fragment
	vec3 final_color = (diffuse_color + specular_color) * vec3(gl_LightSource[0].diffuse /*light_color*/);
	//final_color = vec3(zdist,zdist,zdist)*0.2;

	// add linear fog
//	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	// output color is a mix between fog and final color
//	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), 1.0);
	gl_FragColor = vec4( vec3(final_color), 1.0);

}
