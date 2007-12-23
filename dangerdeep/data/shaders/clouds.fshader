uniform sampler2D tex_cloud;

varying vec2 texcoord;
varying vec3 lightdir;
varying vec3 lightcolor;
varying vec3 viewerdir; /* we need it for the rim lighting/blend */
varying float horizon_alpha;

const float density = 0.25; /* 0.0 = full density */
const float density_rcp = 1.33333; /* 1/(1-density) */

void main()
{
	/* viewer vector */
	vec3 E = normalize(viewerdir);

	/* get and normalize vector to light source */
	vec3 L = normalize(vec3(lightdir.x, lightdir.y, -0.1));

	const float d = 0.00390625; /* 1.0/256.0 */
	const float zh = -0.5;

	/* tex lookups */
	float hr = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(d, 0.0)).x;
	float hu = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(0.0, d)).x;
	float hl = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(-d, 0.0)).x;
	float hd = texture2D(tex_cloud, texcoord.xy*1.0 + vec2(0.0, -d)).x;
	float hr2 = texture2D(tex_cloud, texcoord.xy*2.0 + vec2(d, 0.0)).x;
	float hu2 = texture2D(tex_cloud, texcoord.xy*2.0 + vec2(0.0, d)).x;
	float hl2 = texture2D(tex_cloud, texcoord.xy*2.0 + vec2(-d, 0.0)).x;
	float hd2 = texture2D(tex_cloud, texcoord.xy*2.0 + vec2(0.0, -d)).x;

	/* retrieve and normalize surface normals */
	vec3 N = normalize(vec3(hl - hr + (hl2 - hr2) * 0.5,
				hd - hu + (hd2 - hu2) * 0.5, zh)); /* *0.5 + 0.5 */

	float alpha = (hr-density) * density_rcp * horizon_alpha;

	/* If we use wrapped diffuse angle instead of Lambertian diffuse
	  then we can have the light "wrap" around the surface, giving the 
	  idea of some light scattering over the surface borders. The angle
	  (in radians) for which the light to "spill" over, is going to be
	  controlled by the light+viewer vector to normal angle, we'll just
	  store it, to avoid redoing the calculations later */
	float ldotn = dot( normalize(L+E), N);

	/* Lambertian, in case you want to try (well, it would be Lambertian
	  if the angle considered was between L and N, but we're using L+E.. */
//	float c = clamp( ldotn, 0.0, 1.0);

	/* Wrapped diffuse, see comments above, i'll leave it commented out
	  for you to try both */
	/* are we facing the L+E vector?*/
	float c = (ldotn < 0.0) ? 1.0-min(1.0, acos(ldotn)/1.5707) : /* 90° */
				1.0-min(1.0, acos(ldotn)/radians(90.0+(90.0*abs(ldotn))));

	/* Getting the rim lighting, from the L-E vector */
	float rim = smoothstep(0.25, 0.75, 1.0-dot(E-L, N));
	
	/* base cloud color, always multiplied by light color */
	const vec3 cloudcol = vec3(.86, .85, .89) * lightcolor;
	
	/* mixing C*lightcolor, with cloudcolor, with rim factor blend */
	vec3 final_color = mix( vec3(c, c, c)*lightcolor, cloudcol, rim);

	/* We can improve this later, since clouds seem to behave with different
	  color at horizon, we might use fog to do some tricks later, but for
	  the time being, let's leave fog disabled */
	float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * 
						gl_Fog.scale, 0.0, 1.0);

	/* final color, mix between fog and final color, but see above notes */
//	gl_FragColor = vec3( mix( vec3(gl_Fog.color), final_color, fog_factor),
//						t.w);
	
	gl_FragColor = vec4( final_color, alpha);
}
