// -*- mode: C; -*-

uniform sampler2D tex_y;
uniform sampler2D tex_uv;

varying vec2 texcoord0;

void main()
{
	vec3 YUV;
	YUV.x = texture2D(tex_y, texcoord0).x;
	YUV.yz = texture2D(tex_uv, texcoord0).ra;

	YUV -= vec3(16.0/256.0, 0.5, 0.5);
	const vec3 m0 = vec3(1.164,  0.0, 1.596);
	const vec3 m1 = vec3(1.164, -0.391, -0.813);
	const vec3 m2 = vec3(1.164,  2.018, 0.0);
	vec3 RGB = vec3(dot(m0, YUV), dot(m1, YUV), dot(m2, YUV));
	RGB = clamp(RGB, 0.0, 1.0);

	gl_FragColor = vec4(RGB, 1.0);
}
