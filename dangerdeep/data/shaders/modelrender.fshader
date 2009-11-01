// -*- mode: C; -*-

#ifdef USE_COLORMAP
uniform sampler2D tex_color;	// (diffuse) color map, RGB
#endif
#ifdef USE_NORMALMAP
uniform sampler2D tex_normal;	// normal map, RGB
#endif
#ifdef USE_SPECULARMAP
uniform sampler2D tex_specular;	// (if existent) specular map, LUMINANCE
#endif

#ifdef USE_COLORMAP
varying vec2 texcoord;
#else
varying vec4 color;
#endif
#ifndef USE_NORMALMAP
varying vec3 normal;
#endif
varying vec3 lightdir, halfangle;

#ifdef USE_CAUSTIC
varying vec2 caustic_texcoord;
uniform sampler2D tex_caustic; // (if existent) caustic map, LUMINANCE
#endif


void main()
{
	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

#ifdef USE_NORMALMAP
	// get and normalize normal vector from texmap
	vec3 N = normalize(vec3(texture2D(tex_normal, texcoord.xy)) * 2.0 - 1.0);
#else
	vec3 N = normalize(normal);
#endif

	// compute specular color
	// get and normalize half angle vector
	vec3 H = normalize(halfangle);

	// compute resulting specular color
	vec3 specular_color = vec3(gl_FrontMaterial.specular) *
		pow(max(dot(H, N), 0.0), gl_FrontMaterial.shininess);

#ifdef USE_COLORMAP
	// compute diffuse color
	vec3 diffuse_color = vec3(texture2D(tex_color, texcoord.xy));
#else
	vec3 diffuse_color = color.xyz;
#endif

	// handle ambient
	diffuse_color = diffuse_color * mix(max(dot(L, N), 0.0), 1.0, gl_LightSource[0].ambient.r);

#ifdef USE_SPECULARMAP
	specular_color = specular_color * texture2D(tex_specular, texcoord.xy).x;
#endif

	// final color of fragment
	vec3 final_color = (diffuse_color + specular_color) * vec3(gl_LightSource[0].diffuse /*light_color*/);
#ifdef USE_CAUSTIC
	final_color *= max(texture2D(tex_caustic, caustic_texcoord.xy).x *2.0, 0.5);
#endif

	// add linear fog
//	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	// output color is a mix between fog and final color
#ifdef USE_COLORMAP
	float alpha = 1.0;
#else
	float alpha = color.a;
#endif
//	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), alpha);
	gl_FragColor = vec4( vec3(final_color), alpha);

/* fog:
	// maybe the vertex shader needs to do this: no, it doesn't. It works without that command.
	gl_FogFragCoord = gl_FogCoord;

	// fragment shader:
	fog = (gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale; // linear

	fog = exp2(-gl_Fog.density * gl_FogFragCoord * 1.442695); // exponential

	fog = exp2(-gl_Fog.density * gl_FogDensity
           * gl_FogFragCoord * gl_FogFragCoord * 1.442695); // more exponential

	   // after that: clamp and mix.
*/
}
