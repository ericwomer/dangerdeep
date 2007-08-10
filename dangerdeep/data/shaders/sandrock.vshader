// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal
*/

varying vec2 texcoord0;
varying vec2 texcoord1;
varying vec2 texcoord2;
varying vec2 texcoord3;
varying float grassfac;
varying float diffcol;

void main()
{
	// grass only where normal.z >= 0.95
	const float grass_limit = 0.95;
	const float grass_limit_scale = 1.0 / (1.0 - grass_limit);
	grassfac = (max(gl_Normal.z, grass_limit) - grass_limit) * grass_limit_scale;
	float f = 1.0-grassfac;
	grassfac = 1.0-f*f*f;// grassfac*grassfac;

	// sand rock tex coords, for x any pseudorandom, for y: use height (z)
	texcoord0 = vec2(0.0, gl_Vertex.z * (1.0/128.0));

	// noise texcoord 0: just x-coord/z-coord
	texcoord1 = vec2(gl_Vertex.x, gl_Vertex.z) * (1.0/13.0);

	// noise texcoord 1: just y-coord/z-coord
	texcoord2 = vec2(gl_Vertex.y, gl_Vertex.z) * (1.0/14.0);

	// grass texcoord: just x/y coord scaled
	texcoord3 = vec2(gl_Vertex.x, gl_Vertex.y) * (1.0/16.0);

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);

	// compute light brightness
	diffcol = dot(lightdir_obj, gl_Normal) * 0.75 + 0.25;

	// finally compute position
	gl_Position = ftransform();

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
