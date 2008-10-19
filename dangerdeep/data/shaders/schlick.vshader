
varying vec2 texcoord;
varying vec3 lightdir, viewdir, reflectdir;
attribute vec3 tangentx;

void main()
{
	vec3 normal = normalize( gl_Normal );
	vec3 tangent = normalize( tangentx );
	vec3 binormal = cross( normal, tangent );

	/* gl_Vertex = object space already. Convert light position to object
	   space, and incident vector (object space). */
	vec3 viewer = normalize( vec3( gl_ModelViewMatrixInverse[3]) - 
			vec3(gl_Vertex));
	vec3 tmpvec = normalize( vec3(gl_ModelViewMatrixInverse *
				gl_LightSource[0].position) - vec3(gl_Vertex) );
	vec3 reflectiondir = normalize(reflect( tmpvec, gl_Normal ));

	/* convert incident vector to tangent space */
	lightdir.x = dot(tmpvec, tangent);
	lightdir.y = dot(tmpvec, binormal);
	lightdir.z = dot(tmpvec, normal);

	/* convert viewer to tangent space */
	tmpvec = viewer;
	viewdir.x = dot(tmpvec, tangent);
	viewdir.y = dot(tmpvec, binormal);
	viewdir.z = dot(tmpvec, normal);

	tmpvec = reflectiondir;
	reflectdir.x = dot(tmpvec, tangent);
	reflectdir.y = dot(tmpvec, binormal);
	reflectdir.z = dot(tmpvec, normal);

	/* compute texture coordinates */
	texcoord = gl_MultiTexCoord0.xy;

	/* compute position */
	gl_Position = ftransform();

	/* Set fog coordinates */
	gl_FogFragCoord = gl_Position.z;
}
