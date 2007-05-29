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

	// storing dot products, for fresnel, rescaling, etc..
	float doten = dot(E, N);
	float dotln = dot(L, N);
	// and we need the half vector for blinn speculars
	vec3 H = normalize(L+E);

	// now for the specular term that we're going to add to the 
	// reflection color later, blinn model, phong produces 
	// incorrect results at grazing angles.
	vec3 specular_color = vec3(gl_LightSource[0].diffuse)
		* pow(max(0.0, dot(N, H)), 140.0) * 6.0; // 140.0 = water_shininess

	// fresnel term approximation
	float fresnel = clamp(doten, 0.0, 1.0) + 1.0;
	// approximation for fresnel term is 1/((x+1)^8)
	// using pow() seems a little bit faster (Geforce5700)
	fresnel = pow(fresnel, -8.0);

	// compute reflection and refraction colors
	float dl = max(dotln*dotln * 0.6 + doten*doten * 0.8, 0.0);
	vec3 reflectioncol = vec3(gl_Color) * dl;
	// light blue mix with sun... fixme
	vec3 refractioncol = vec3(gl_LightSource[0].diffuse) * vec3(0.4, 0.59, 0.79) * dl;

	// we're going to multiply by viewer to surface angle
	// mix reflection and refraction (upwelling) color, and add specular color
	vec3 water_color = mix(refractioncol, reflectioncol, fresnel)
		+ (specular_color * max(doten, 0.2));

	// add linear fog
	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), water_color, fog_factor), 1.0);
}
