// -*- mode: C; -*-

// fixme: fog!
// fixme: maybe lookup texmap is faster than pow(). Quick tests showed that this is not the case...

/* input:
texture[0], 2D		- (diffuse) color map, RGB
texture[1], 2D		- normal map, RGB
texture[2], 2D		- (if existent) specular map, LUMINANCE
*/

// fixme: how to name texture units here?!
uniform sampler2D tex_color;
uniform sampler2D tex_normal;
uniform sampler2D tex_specular;

varying vec2 outtexcoorddiff, outtexcoordnrml;	// fixme: one parameter is enough
varying vec3 lightdir, halfangle;

void main()
{
	// get and normalize vector to light source
	vec3 L = normalize(lightdir);

	// get and normalize normal vector from texmap
	vec3 N = normalize(vec3(texture2D(tex_normal, outtexcoordnrml)) * 2.0 - 1.0);

	// compute specular color
	// get and normalize half angle vector
	vec3 H = normalize(halfangle);

	// compute resulting specular color
	vec3 specular_color = vec3(gl_FrontMaterial.specular) *
		pow(max(dot(H, N), 0.0), gl_FrontMaterial.shininess);

	// compute diffuse color
	vec3 diffuse_color = vec3(texture2D(tex_color, outtexcoorddiff));

	// handle ambient
	diffuse_color = diffuse_color * mix(max(dot(L, N), 0.0), 1.0, gl_LightSource[0].ambient.r);

	//#ifdef USE_SPECULARMAP
	specular_color = specular_color * texture2D(tex_specular, outtexcoordnrml).x;
	//#endif

	gl_FragColor = vec4((diffuse_color + specular_color) * vec3(gl_LightSource[0].diffuse) /*light_color*/, 1);
	//gl_FragColor = vec4(bla, 1);
	//gl_FragColor.a = 1;
}
