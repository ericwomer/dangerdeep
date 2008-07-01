// -*- mode: C; -*-

uniform sampler2D tex_color;	// (diffuse) color map, RGB
//uniform sampler2D tex_normal;	// normal map, RGB   - fixme: try 2 component map (x/y) and compute z instead of normalize

varying vec2 texcoord;
varying vec3 lightdir, halfangle;

void main()
{
	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

	// get and normalize normal vector from texmap
	//vec3 N = normalize(vec3(texture2D(tex_normal, texcoord.xy)) * 2.0 - 1.0);
	vec3 N = vec3(0, 0, 1);

	// compute specular color
	// get and normalize half angle vector
	vec3 H = normalize(halfangle);

	// compute resulting specular color
	vec3 specular_color = vec3(gl_FrontMaterial.specular) *
		pow(max(dot(H, N), 0.0), gl_FrontMaterial.shininess);

	// compute diffuse color
	vec4 col = texture2D(tex_color, texcoord.xy);
	if (col.w < 0.001) discard;
	vec3 diffuse_color = vec3(col);

	// handle ambient
	diffuse_color = diffuse_color * mix(max(dot(L, N), 0.0), 1.0, gl_LightSource[0].ambient.r * 0.25 + 0.75);

	// final color of fragment
	vec3 final_color = (diffuse_color + specular_color) * vec3(gl_LightSource[0].diffuse /*light_color*/);

	// output color is a mix between fog and final color
	gl_FragColor = vec4( vec3(final_color), col.w);
}
