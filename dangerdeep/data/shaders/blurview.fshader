// -*- mode: C; -*-

varying vec2 texc_0;
varying vec2 texc_1;

uniform sampler2D tex_view;
uniform sampler2D tex_blur;

// we can achieve more blur by using larger offsets here.
// default 1/512 and 2/512, better 3/512 and 5/512.
// effect could be even better with some more random offsets.
const float tex_res_rcp = 3.0/512.0;
const float tex_res_rcp2 = 5.0/512.0;
const float mix = 1.0/12.0;

void main()
{
	vec3 c0 = vec3(texture2D(tex_view, texc_0));
	vec3 c1 = vec3(texture2D(tex_view, texc_0 + vec2(0.0, -tex_res_rcp2))) * mix;
	vec3 c2 = vec3(texture2D(tex_view, texc_0 + vec2(-tex_res_rcp, -tex_res_rcp))) * mix;
	vec3 c3 = vec3(texture2D(tex_view, texc_0 + vec2(0.0, -tex_res_rcp))) * mix;
	vec3 c4 = vec3(texture2D(tex_view, texc_0 + vec2(tex_res_rcp, -tex_res_rcp))) * mix;
	vec3 c5 = vec3(texture2D(tex_view, texc_0 + vec2(-tex_res_rcp2, 0.0))) * mix;
	vec3 c6 = vec3(texture2D(tex_view, texc_0 + vec2(-tex_res_rcp, 0.0))) * mix;
	vec3 c7 = vec3(texture2D(tex_view, texc_0 + vec2(tex_res_rcp, 0.0))) * mix;
	vec3 c8 = vec3(texture2D(tex_view, texc_0 + vec2(tex_res_rcp2, 0.0))) * mix;
	vec3 c9 = vec3(texture2D(tex_view, texc_0 + vec2(-tex_res_rcp, tex_res_rcp))) * mix;
	vec3 cA = vec3(texture2D(tex_view, texc_0 + vec2(0.0, tex_res_rcp))) * mix;
	vec3 cB = vec3(texture2D(tex_view, texc_0 + vec2(tex_res_rcp, tex_res_rcp))) * mix;
	vec3 cC = vec3(texture2D(tex_view, texc_0 + vec2(0.0, tex_res_rcp2))) * mix;
	float mixfac = texture2D(tex_blur, texc_1).x;
	vec3 csum = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + cA + cB + cC;
	//vec3 c = mix(c0, csum, mixfac);
	vec3 c = csum * mixfac + c0 * (1.0-mixfac);
	gl_FragColor = vec4(c, 1.0);
}
