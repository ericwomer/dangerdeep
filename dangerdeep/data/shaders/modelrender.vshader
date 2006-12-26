// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Normal		(tangentz)
   gl_MultiTexCoord0	(texcoord)
   gl_MultiTexCoord1	(tangentx)
   gl_Color		(r/x value has righthanded sign).
*/

/* can we give names to default attributes? or do we need to use named attributes for that?
   how to give named attributes with vertex buffer objects?
   we could assign them to a variable, but would that be efficient? an unnecessary copy.
   but the shader compiler should be able to optimize that...
   the way should be to use vertex attributes (together with vertex arrays or VBOs),
   that have a name and use that name here.
   until then, access sources directly or via a special variable.
*/

varying vec2 outtexcoorddiff, outtexcoordnrml;
varying vec3 lightdir, halfangle;

void main()
{
	// compute tangent space
	// gl_Color.x = righthanded-info (need to transform from 0|1 -> -1|1
	// gl_Normal = tangentz
	// gl_MultiTexCoord1 = tangentx
	vec3 tangenty = cross(gl_Normal, vec3(gl_MultiTexCoord1)) * (gl_Color.x * 2.0 - 1.0);

	// compute direction to light in object space (L)
	// light.position.w is 0 or 1, 0 for directional light, 1 for point light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);

	// direction to viewer (E)
	// compute direction to viewer (E) in object space (mvinv*(0,0,0,1) - inputpos)
	vec3 viewerdir_obj = normalize(vec3(gl_ModelViewMatrixInverse[3]) - vec3(gl_Vertex));

	// compute halfangle vector (H = ||L+E||)
	vec3 halfangle_obj = normalize(viewerdir_obj + lightdir_obj);

	// transform light direction to tangent space
	lightdir.x = dot(gl_MultiTexCoord1.xyz /*tangentx*/, lightdir_obj);
	lightdir.y = dot(tangenty, lightdir_obj);
	lightdir.z = dot(gl_Normal /*tangentz*/, lightdir_obj);

	halfangle.x = dot(gl_MultiTexCoord1.xyz /*tangentx*/, halfangle_obj);
	halfangle.y = dot(tangenty, halfangle_obj);
	halfangle.z = dot(gl_Normal /*tangentz*/, halfangle_obj);

	// compute texture coordinates
	//fixme: something is wrong about texture matrices.
	// wasted, most models have ONE texture matrix for both maps.
	outtexcoorddiff = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
	outtexcoordnrml = (gl_TextureMatrix[1] * gl_MultiTexCoord0).xy;

	// finally compute position
	gl_Position = ftransform();
}
