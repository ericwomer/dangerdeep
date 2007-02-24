// -*- mode: C; -*-

varying vec3 viewerdir, halfangle, lightdir;
varying vec2 noisetexcoord;
varying vec4 reflectiontexcoord;	// x,y,w
varying vec4 foamamounttexcoord;	// x,y,w
varying float crest_foam;

uniform vec3 viewpos;
uniform vec3 upwelltop;
uniform vec3 upwellbot;
uniform vec3 upwelltopbot;

const float foamamount_f1 = 0.8;
const float foamamount_f2 = -1.0;
const vec3 yaxis = { 0.0, 1.0, 0.0 };
const float virtualplane_height = 12.0;

void main()
{
	// normalize vertex normal
	vec3 N = normalize(gl_Normal);

	// compute upwelling color (slope dependent)
	// that is (inputpos.z + viewpos.z + 3) / 9 + (normal.z - 0.8);
	// or (inputpos.z + viewpos.z) / 9 + normal.z + 1/3 - 0.8
	// or (inputpos.z + viewpos.z) * 1/9 + normal.z - 7/15
	// 1/9 = 0.1111111 , 7/15 = 0.466667
	gl_FrontColor.xyz = upwelltopbot *
		clamp((gl_Vertex.z + viewpos.z) * 0.1111111 + N.z - 0.4666667, 0.0, 1.0) + upwellbot;

	// compute amount of foam as outputcolor.alpha from inputpos.z
	crest_foam = clamp((gl_Vertex.z + viewpos.z) * foamamount_f1 + foamamount_f2, 0.0, 1.0);

	// compute tangent space.
	// tangentx = (0,1,0).cross(normal), because v coordinate
	// increases with vertex y coordinate => tangentx is (nz, 0, -nx)
	vec3 tangentx = normalize(cross(yaxis, N));
	vec3 tangenty = cross(N, tangentx);

	// compute direction to viewer (E) in object space (mvinv*(0,0,0,1) - inputpos)
	vec3 viewerdir_obj = normalize(vec3(gl_ModelViewMatrixInverse[3]) - vec3(gl_Vertex));
	// transform viewerdir to tangent space
	viewerdir.x = dot(viewerdir_obj, tangentx);
	viewerdir.y = dot(viewerdir_obj, tangenty);
	viewerdir.z = dot(viewerdir_obj, N);

	// transform vertex to projection space (clip coordinates)
	gl_Position = ftransform();

	// compute halfangle between direction to viewer and direction to light
	// transform light pos to object space. we assume mvinv has no projection coefficients.
	// light is directional, so use dot3, H = ||L + E||
	vec3 lightdir_obj = normalize(vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position));
	vec3 halfangle_obj = normalize(lightdir_obj + viewerdir_obj);
	// transform half angle to tangent space
	halfangle.x = dot(halfangle_obj, tangentx);
	halfangle.y = dot(halfangle_obj, tangenty);
	halfangle.z = dot(halfangle_obj, N);
	// transform lightdir to tangent space
	lightdir.x = dot(lightdir_obj, tangentx);
	lightdir.y = dot(lightdir_obj, tangenty);
	lightdir.z = dot(lightdir_obj, N);

	// transform inputpos.xy with texture matrix to get texture coodinates
	noisetexcoord = (gl_TextureMatrix[0] * gl_Vertex).xy;

	// compute reflection texture coordinates
	// formula to compute them from inputpos (coord):
	// vector3f texc = coord + N * (VIRTUAL_PLANE_HEIGHT * N.z);
	// texc.z -= VIRTUAL_PLANE_HEIGHT;
	// uv1[ptr] = texc;
	// after that mulitply with texture matrix!
	vec3 texc = N * (virtualplane_height * N.z) + vec3(gl_Vertex);
	texc.z -= virtualplane_height;
	reflectiontexcoord = gl_TextureMatrix[1] * vec4(texc, 1.0);

	// compute texture coordinates for foam-amount texture
	foamamounttexcoord = gl_TextureMatrix[1] * vec4(vec2(gl_Vertex), vec2(-viewpos.z, 1.0));
}
