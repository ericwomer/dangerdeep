
varying vec2 texcoord;
varying vec3 lightdir, viewdir, reflectdir;
attribute vec4 tangentx_righthanded;

void main()
{
	/* Build tangent space */
	vec3 tangentx = vec3( tangentx_righthanded );
	vec3 tangenty = cross( gl_Normal, tangentx) * tangentx_righthanded.w;

	/* Build incident vector */
	vec3 lightpos_obj = vec3( gl_ModelViewMatrixInverse *
						gl_LightSource[0].position);
	vec3 lightdir_obj = normalize( lightpos_obj - vec3(gl_Vertex) *
						gl_LightSource[0].position.w);

	/* Build viewer vector */
	vec3 viewerdir_obj = normalize( vec3( gl_ModelViewMatrixInverse[3]) -
						vec3(gl_Vertex));

	/* Build reflection vector */
	vec3 reflection_obj = normalize( reflect( -lightdir_obj, gl_Normal) );

	/* Convert incident and viewer vectors to tangent space */
	lightdir.x = dot(tangentx, lightdir_obj);
	lightdir.y = dot(tangenty, lightdir_obj);
	lightdir.z = dot(gl_Normal /*tangentz*/, lightdir_obj);

	viewdir.x = dot(tangentx, viewerdir_obj);
	viewdir.y = dot(tangenty, viewerdir_obj);
	viewdir.z = dot(gl_Normal /*tangentz*/, viewerdir_obj);

	reflectdir.x = dot(tangentx, reflection_obj);
	reflectdir.y = dot(tangenty, reflection_obj);
	reflectdir.z = dot(gl_Normal, reflection_obj);


	/* Set texture coordinates */
	texcoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;

	/* Set position */
	gl_Position = ftransform();

	/* Set fog coordinates */
	gl_FogFragCoord = gl_Position.z;
}
