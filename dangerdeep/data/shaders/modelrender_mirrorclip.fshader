// -*- mode: C; -*-

// fixme: fog!

uniform sampler2D tex_color;	// diffuse color map (RGB)

varying vec2 texcoord;

// fixme: need to declare that?
// varying vec4 gl_Color;

void main()
{
	// compute diffuse color
	vec3 diffuse_color = vec3(texture2D(tex_color, texcoord.xy));

	gl_FragColor = vec4(diffuse_color * vec3(gl_Color), 1);
}
