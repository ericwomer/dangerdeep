// -*- mode: C; -*-
#version 110

// it seems we can define at max. 8 varying parameters...
varying vec3 viewerdir;
varying vec3 lightdir;
varying vec3 normal;
varying vec3 foamtexcoord; // z coord is crest_foam value
#ifdef HQ_SFX
varying vec3 realcoordinates;
#else
varying vec4 reflectiontexcoord;	// x,y,w
#endif
varying vec4 foamamounttexcoord;	// x,y,w
varying vec2 noise_texc_0;
varying vec2 noise_texc_1;

uniform vec3 viewpos;
uniform vec3 upwelltop;
uniform vec3 upwellbot;
uniform vec3 upwelltopbot;
uniform vec3 noise_xform_0;
uniform vec3 noise_xform_1;

attribute float amount_of_foam;

const float foamamount_f1 = 0.8;
const float foamamount_f2 = -1.0;
#ifndef HQ_SFX
const float virtualplane_height = 12.0;
#endif

void main()
{
	// normalize vertex normal
	vec3 N = normalize(gl_Normal);
	normal = N;

	// compute upwelling color (slope dependent)
	// that is (inputpos.z + viewpos.z + 3) / 9 + (normal.z - 0.8);
	// or (inputpos.z + viewpos.z) / 9 + normal.z + 1/3 - 0.8
	// or (inputpos.z + viewpos.z) * 1/9 + normal.z - 7/15
	// 1/9 = 0.1111111 , 7/15 = 0.466667
	gl_FrontColor.xyz = upwelltopbot *
		clamp((gl_Vertex.z + viewpos.z) * 0.1111111 + N.z - 0.4666667, 0.0, 1.0) + upwellbot;

	// compute direction to viewer (E) in object space (mvinv*(0,0,0,1) - inputpos)
	// Note! Do NOT normalize it or weird values will result for the large horizon faces!
	//viewerdir = normalize(vec3(gl_ModelViewMatrixInverse[3]) - vec3(gl_Vertex));
	viewerdir = vec3(gl_ModelViewMatrixInverse[3]) - vec3(gl_Vertex);

	// transform vertex to projection space (clip coordinates)
	gl_Position = ftransform();

	// set fog coordinate
	// looks worse, as fog color is mainly taken from sun color, and this is wrong
	// at sunrise etc.
	//gl_FogFragCoord = gl_Position.z;

	// transform light pos to object space. we assume mvinv has no projection coefficients.
	// light is directional, so lightpos.w = 0
	lightdir = normalize(vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position));

	// transform noise coordinates
	noise_texc_0 = vec2(gl_Vertex) * noise_xform_0.z + noise_xform_0.xy;
	noise_texc_1 = vec2(gl_Vertex) * noise_xform_1.z + noise_xform_1.xy;

	// transform inputpos.xy with texture matrix to get texture coodinates
	//fixme: use uniforms here as well, no tex matrix.
	foamtexcoord = vec3((gl_TextureMatrix[0] * gl_Vertex).xy, amount_of_foam);

#ifdef HQ_SFX
	realcoordinates = vec3(gl_Vertex);
#else
	// compute reflection texture coordinates
	// formula to compute them from inputpos (coord):
	// vector3f texc = coord + N * (VIRTUAL_PLANE_HEIGHT * N.z);
	// texc.z -= VIRTUAL_PLANE_HEIGHT;
	// uv1[ptr] = texc;
	// after that mulitply with texture matrix!
	vec3 texc = N * (virtualplane_height * N.z) + vec3(gl_Vertex);
	texc.z -= virtualplane_height;
	reflectiontexcoord = gl_TextureMatrix[1] * vec4(texc, 1.0);
#endif

	// compute texture coordinates for foam-amount texture
	foamamounttexcoord = gl_TextureMatrix[1] * vec4(vec2(gl_Vertex), vec2(-viewpos.z, 1.0));
}
