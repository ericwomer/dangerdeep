// -*- mode: C; -*-
#version 110

varying vec3 viewerdir;
varying vec3 lightdir;
varying vec3 normal;
varying vec3 foamtexcoord;
#ifdef HQ_SFX
varying vec3 realcoordinates;
#else
varying vec4 reflectiontexcoord;	// x,y,w
#endif
varying vec4 foamamounttexcoord;	// x,y,w
varying vec2 noise_texc_0;
varying vec2 noise_texc_1;

uniform sampler2D tex_normal;		// normal map, RGB
uniform sampler2D tex_reflection;	// reflection, RGB
uniform sampler2D tex_foam;		// foam texture (tileable)
uniform sampler2D tex_foamamount;	// amount of foam per pixel

#ifdef HQ_SFX
uniform mat4 reflection_mvp;
const float virtualplane_height = 12.0;
#endif

const float water_shininess = 140.0;

void main()
{
	// compute normal vector
	vec3 N0 = vec3(texture2D(tex_normal, noise_texc_0) * 2.0 - 1.0);
	vec3 N1 = vec3(texture2D(tex_normal, noise_texc_1) * 2.0 - 1.0);
	vec3 N = normalize(normal+N0+N1);

	// compute direction to viewer
	vec3 E = normalize(viewerdir);

	// compute direction to light source
	vec3 L = normalize(lightdir);

	// half vector
	vec3 H = normalize(L + E);

	// store E, N dot product, we use it in fresnel and refraction factor
	float doten = dot(E, N);

	// compute fresnel term. we need to clamp because the product could be negative
	// when the angle between face normal and viewer is nearly perpendicular.
	// using abs gives ugly ring-like effects, but direct clamp leads to large areas with maximum
	// reflectivity. We need to tweak E in the vertex shader, so that this term can't be negative,
	// as work-around... fixme.
	// IDEA: if we would use a texture map as lookup for the fresnel term, we can avoid this:
	// - texture lookup could be faster!
	// - use a texture with clamping, but allow coordinate < 0 (by scaling, then clamp)
	// - fresnel term is 1/((x+1)^8), so fresnel(0)=1, BUT: fresnel(x) for x < 0,
	//   is erratic/noisy (smoothed) between 0 and 1, simulating rough water.
	//   That way the rings disappear,
	//   and are replaced by additional surface detail.
	// new result: without the abs() around dot(), it doesn't look worse. so keep out the abs()
	// this also means the case that E*N < 0 is not so problematic. So we don't need the lookup
	// texture.
	float fresnel = clamp(doten, 0.0, 1.0) + 1.0;
	// approximation for fresnel term is 1/((x+1)^8)
#if 1
	// using pow() seems a little bit faster (Geforce5700)
	fresnel = pow(fresnel, -8.0) * 0.8;
#else
	fresnel = fresnel * fresnel;	// ^2
	fresnel = fresnel * fresnel;	// ^4
	fresnel = fresnel * fresnel;	// ^8
	fresnel = (1.0/fresnel) * 0.8;	// never use full reflectivity, at most 0.8
	// that multiplication greatly increases the realism of the appearance!
#endif

	// possible optimization: make a 2d lookup texture, one dimension with fresnel term,
	// second dimension with specular term. Spares two pow() instructions. But we already used
	// all four texture units. Doubtful that performance would be higher...

	// compute specular light brightness (blinn shading with specular falloff/size control)
	vec3 specular_color = vec3(gl_LightSource[0].diffuse)
		* pow(max(0.0, dot(N, H)), water_shininess) * 6.0;

	// compute refraction color
	// float dl = max( dot( L N), 0.0);
	float dl = max(dot(L, N) * abs(1.0 - max(doten, 0.2)), 0.0);
	vec3 refractioncol = vec3(gl_Color) * dl;

	// mix reflection and refraction (upwelling) color, and add specular color
#ifdef HQ_SFX
	vec3 texc = N * (virtualplane_height * N.z) + realcoordinates;
	texc.z -= virtualplane_height;
	vec4 reflectiontexcoord = reflection_mvp * vec4(texc, 1.0);
#endif
	vec3 reflectioncolor = vec3(texture2DProj(tex_reflection, reflectiontexcoord));
	vec3 water_color = mix(refractioncol, reflectioncolor, fresnel) + specular_color;

	// fetch amount of foam, sum of texture and cresnel foam, multiplied with luminance
	// from foam texmap
	float foam_amount = min(texture2DProj(tex_foamamount, foamamounttexcoord).x
				+ foamtexcoord.z /*crest_foam*/, 1.0)
		* texture2D(tex_foam, foamtexcoord.xy).x;

	vec3 final_color = mix(water_color, vec3(gl_LightSource[0].diffuse), foam_amount);

	// add linear fog
//	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0.0, 1.0);
	float fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord * 1.4426940);

	// output color is a mix between fog and final color
	gl_FragColor = vec4(mix(reflectioncolor, final_color, fog_factor), 1.0);
}
