#define IOR	2.5	/* IOR for metal */
#define ROUGHNESS	0.3	/* Apparent surface roughness, 0 = smooth, 1 = rough */

uniform sampler2D tex_color;	/* Color texture */
uniform sampler2D tex_normal;	/* Normal texture */


varying vec2 texcoord;
varying vec3 lightdir, viewdir, reflectdir;

void main()
{
	/* normalize incident and viewer vector */
	vec3 Ln = normalize(lightdir);
	vec3 Vf = -normalize(viewdir);

	/* get and normalize normal vector from texmap */
	vec3 Nn = normalize(vec3(texture2D(tex_normal, texcoord.xy)) * 2.0 - 1.0);

	/* get half vector */
	vec3 Rn = normalize(reflectdir);

	/* store preset quantities */
	float cosalpha = dot( Nn, Vf );
	float costheta = dot( Ln, Nn );
	float cospsi = max( 0.0, dot( Rn, Nn ) );

	/* Fresnel term to scale the specular with (Kr, 1-Kr for diffuse) */
	float kr = (IOR - 1.0) / (IOR + 1.0);
	kr = kr * kr;
	kr = kr + (1.0 - kr) * pow( 1.0 - cosalpha, 5.0 );

	/* Specular term, Schlick's fast BRDF */
	float nr = 1.0 / (ROUGHNESS * ROUGHNESS);
	float coeff = cospsi / (nr - nr * cospsi * cospsi);
	
	/* Diffuse term */
	vec3 Cdiff = mix( max( costheta, 0.0), 1.0,
					gl_LightSource[0].ambient.r) * vec3( texture2D(
				tex_color, texcoord.xy));

	/* Specular term */
//	Cspec *= texture2D( tex_specular, texcoord.xy).x;
//	vec3 Cspec = coeff * vec3( texture2D(tex_color, texcoord.xy).x);
//	vec3 Cspec = coeff * vec4(texture2D(tex_color, texcoord.xy).a;
	float specularity = texture2D(tex_color, texcoord.xy).w;
	vec3 Cspec = vec3(coeff * specularity);

	/* Total color */
	vec3 Ctotal = clamp( vec3( gl_LightSource[0].diffuse )
			* (Cdiff + Cspec), 0.0, 1.0 );

	/* Add linear fog */
	float fogfactor = clamp( (gl_Fog.end - gl_FogFragCoord) *
						gl_Fog.scale, 0.0, 1.0 );

	/* Output total mixed with fog */
	gl_FragColor = vec4( mix( vec3(gl_Fog.color), Ctotal, fogfactor), 1.0);
}
