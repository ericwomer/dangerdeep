// -*- mode: C; -*-

uniform sampler2D tex_color;	// (diffuse) color map, RGB
uniform sampler2D tex_normal;	// normal map, RGBA

varying vec2 texcoord;
varying vec3 lightdir, halfangle;
varying vec3 viewerpos;

const float depth_factor = 0.05;

float ray_intersect(vec2 dp, vec2 ds)
{
	const int linear_search_steps = 20;
	float size = 1.0 / float(linear_search_steps);
	float depth = 0.0;
	float best_depth = 1.0;

	for(int i = 0; i < linear_search_steps - 1; ++i) {
		depth += size;
		float t = texture2D(tex_normal, dp + ds * depth).a;
		// fixme: is "break" here faster?
		if(best_depth > 0.996)
			if(depth >= t)
				best_depth = depth;
	}
	depth = best_depth;

	const int binary_search_steps = 5;
	for(int i = 0; i < binary_search_steps; ++i) {
		size *= 0.5;
		float t = texture2D(tex_normal, dp + ds * depth).a;
		if(depth >= t) {
			best_depth = depth;
			depth -= 2.0 * size;
		}
		depth += size;
	}

	return best_depth;
}



void main()
{
#if 1
	// compute new texcoord
	vec3 V = normalize(viewerpos);
	float a = V.z;	// why not negate?!
	vec2 s = V.xy;
	s *= depth_factor / a;
	s.x = -s.x;	// why that?!
	vec2 ds = s;
	vec2 dp = texcoord.xy;
	float d = ray_intersect(dp, ds);
	vec2 texcoord_new = dp + ds * d;
#else
	vec2 texcoord_new = texcoord.xy;
#endif

	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

	// get and normalize normal vector from texmap
	vec3 N = normalize(vec3(texture2D(tex_normal, texcoord_new)) * 2.0 - 1.0);

	// compute specular color
	// get and normalize half angle vector
	vec3 H = normalize(halfangle);

#if 0
	// compute resulting specular color
	vec3 specular_color = vec3(gl_FrontMaterial.specular) *
		pow(max(dot(H, N), 0.0), gl_FrontMaterial.shininess);
#else
	vec3 specular_color = vec3(0.0, 0.0, 0.0);
#endif

	// compute diffuse color
	vec3 diffuse_color = vec3(texture2D(tex_color, texcoord_new));

	// handle ambient
	diffuse_color = diffuse_color * mix(max(dot(L, N), 0.0), 1.0, gl_LightSource[0].ambient.r);

	// final color of fragment
	vec3 final_color = (diffuse_color + specular_color) * vec3(gl_LightSource[0].diffuse /*light_color*/);

	// add linear fog
//	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	// output color is a mix between fog and final color
//	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), 1.0);
	gl_FragColor = vec4( vec3(final_color), 1.0);

}
