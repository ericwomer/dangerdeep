// -*- mode: C; -*-

// fixme: fog!
// fixme: maybe lookup texmap is faster than pow(). Quick tests showed that this is not the case...

uniform sampler2D tex_color;	// (diffuse) color map, RGB
uniform sampler2D tex_normal;	// normal map, RGB   - fixme: try 2 component map (x/y) and compute z instead of normalize
#ifdef USE_SPECULARMAP
uniform sampler2D tex_specular;	// (if existent) specular map, LUMINANCE
#endif

varying vec2 texcoord;
varying vec3 lightdir, halfangle;

void main()
{
	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

	// get and normalize normal vector from texmap
	vec3 N = normalize(vec3(texture2D(tex_normal, texcoord.xy)) * 2.0 - 1.0);

	// compute specular color
	// get and normalize half angle vector
	vec3 H = normalize(halfangle);

	// compute resulting specular color
	vec3 specular_color = vec3(gl_FrontMaterial.specular) *
		pow(max(dot(H, N), 0.0), gl_FrontMaterial.shininess);

	// compute diffuse color
	vec3 diffuse_color = vec3(texture2D(tex_color, texcoord.xy));

	// handle ambient
	diffuse_color = diffuse_color * mix(max(dot(L, N), 0.0), 1.0, gl_LightSource[0].ambient.r);

#ifdef USE_SPECULARMAP
	specular_color = specular_color * texture2D(tex_specular, texcoord.xy).x;
#endif

	gl_FragColor = vec4((diffuse_color + specular_color) * vec3(gl_LightSource[0].diffuse) /*light_color*/, 1);
}
