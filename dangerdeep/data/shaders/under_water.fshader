// -*- mode: C; -*-
#version 110

varying vec3 viewerdir;
varying vec3 lightdir;
varying vec3 normal;
varying vec2 noise_texc_0;
varying vec2 noise_texc_1;

uniform sampler2D tex_normal;		// normal map, RGB

void main()
{
	// compute normal vector
	vec3 N0 = vec3(texture2D(tex_normal, noise_texc_0) * 2.0 - 1.0);
	vec3 N1 = vec3(texture2D(tex_normal, noise_texc_1) * 2.0 - 1.0);
	vec3 N = normalize(normal+N0+N1);

	// compute direction to viewer
	// L, N etc. are all point upwards, so negate E
	vec3 E = -normalize(viewerdir);

	// compute direction to light source
	vec3 L = normalize(lightdir);

	float fresnel = clamp(dot(E, N), 0.0, 1.0) + 1.0;
	// approximation for fresnel term is 1/((x+1)^8)
	// using pow() seems a little bit faster (Geforce5700)
	fresnel = pow(fresnel, -8.0);

	// compute refraction color
	float doten = dot(E, N);
	float dotln = dot(L, N);
	float dl = max(dotln*dotln * 0.6 + doten*doten * 0.8, 0.0);
	vec3 reflectioncol = vec3(gl_Color) * dl;
	// light blue mix with sun... fixme
	vec3 refractioncol = vec3(gl_LightSource[0].diffuse) * vec3(0.4, 0.59, 0.79) * dl;

	// mix reflection and refraction (upwelling) color, and add specular color
	vec3 water_color = mix(refractioncol, reflectioncol, fresnel);

	// add linear fog
	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), water_color, fog_factor), 1.0);
}
