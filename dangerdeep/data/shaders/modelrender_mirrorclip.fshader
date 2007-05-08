// -*- mode: C; -*-
#version 110

uniform sampler2D tex_color;	// diffuse color map (RGB)

varying vec2 texcoord;

void main()
{
	// compute diffuse color
	vec3 diffuse_color = vec3(texture2D(tex_color, texcoord.xy));

	// add linear fog
	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);

	vec3 final_color = diffuse_color * vec3(gl_Color);

	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(vec3(gl_Fog.color), final_color, fog_factor), 1.0);
}
