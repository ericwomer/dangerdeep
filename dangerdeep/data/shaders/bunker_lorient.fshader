// -*- mode: vim4ever

uniform sampler2D tex_color;	// (diffuse) color map, RGB
uniform sampler2D tex_normal;	// normal map, RGB

varying vec2 texcoord;

varying vec3 lightdir;

void main()
{
	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

	// get and normalize normal vector from texmap
	vec3 N = normalize( vec3( texture2D(tex_normal, texcoord.xy)) * 2.0 - 1.0);

	// compute diffuse color
	vec3 diffuse_color = vec3( texture2D(tex_color, texcoord.xy) );

	// handle ambient
	diffuse_color = diffuse_color * mix( max( dot(L, N), 0.0), 1.0,
			gl_LightSource[0].ambient.r);

	// final color of fragment
	vec3 final_color = diffuse_color * vec3(gl_LightSource[0].diffuse);

	gl_FragColor = vec4( vec3(final_color), 1.0);
}
