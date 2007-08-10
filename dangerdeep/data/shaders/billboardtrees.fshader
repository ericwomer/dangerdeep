// -*- mode: C; -*-

uniform sampler2D textrees;

varying vec2 texcoord0;
varying float diffcol;

void main()
{
	vec4 c0 = texture2D(textrees, texcoord0);
	vec4 final_color = c0 * diffcol;

	// add linear fog
	float fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord * 1.4426940);
	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color.xyz, fog_factor), final_color.a);
}
