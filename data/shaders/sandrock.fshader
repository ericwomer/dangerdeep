// -*- mode: C; -*-

uniform sampler2D texsandrock;
uniform sampler2D texnoise;
uniform sampler2D texgrass;
//uniform sampler2D texnormal;

varying vec2 texcoord0;
varying vec2 texcoord1;
varying vec2 texcoord2;
varying vec2 texcoord3;
varying float grassfac;
varying float diffcol;

void main()
{
	float noisy = (texture2D(texnoise, texcoord1).x * 0.5 +
		       texture2D(texnoise, texcoord2).x * 0.5);
	vec2 texc = texcoord0;
	texc.y += noisy * (1.0/16.0);
//	vec3 col = texture2D(texsandrock, texcoord0).xyz;
	vec3 col = texture2D(texsandrock, texc).xyz;
	vec3 col2 = texture2D(texsandrock, texcoord0).xyz;
	col = col * noisy + col2 * 0.25; // fixme: this is not optimal
//	col = col * 0.5 + vec3(noisy, noisy, noisy) * 0.5;
	vec3 grass = texture2D(texgrass, texcoord3).xyz
		* (texture2D(texnoise, texcoord3 * 2.0).x * 0.5 + 0.5);
	col = mix(col, grass, grassfac);
	vec3 final_color = col * diffcol;

	// add linear fog
	float fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord * 1.4426940);
	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), 1.0);
}
