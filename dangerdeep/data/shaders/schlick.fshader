const float roughness=0.2; /* Apparent surface roughness, 0 = smooth,
							  1 = rough */

uniform sampler2D tex_color;	/* Color texture */
uniform sampler2D tex_normal;	/* Normal texture */

varying vec2 texcoord;
varying vec3 lightdir, viewdir, reflectdir;

void main()
{
	/* normalize incident and viewer vector */
	vec3 Ln = normalize(lightdir); /* passed normalized */
	vec3 Vf = -normalize(viewdir); /* passed normalized */

	/* get and normalize normal vector from texmap */
	vec3 Nn = normalize(vec3(texture2D(tex_normal, texcoord.xy)) * 2.0 - 1.0);
	
	/* get reflection vector */
	vec3 Rn = normalize( reflectdir );

	/* store preset quantities */
	float cosalpha = dot( Nn, Vf );
	float costheta = dot( Ln, Nn );
	float cospsi = max( 0.0, dot( Rn, Vf ) );

	/* Specular term, Schlick's fast BRDF */
	float nr = 1.0 / (roughness * roughness);
	float coeff = cospsi / (nr - nr * cospsi + cospsi);
	
	/* Diffuse term */
	vec4 Cmain = texture2D( tex_color, texcoord.xy );
	vec3 Cdiff = mix( max( 0.0, costheta), 1.0,
					gl_LightSource[0].ambient.r) * Cmain.rgb;

	/* Specular term */
	vec3 Cspec = vec3( coeff * Cmain.a );
	
	/* Total color */
	vec3 Ctotal = vec3(gl_LightSource[0].diffuse) * (Cdiff + Cspec);

	gl_FragColor = vec4( Ctotal, 1.0 );

}
