// -*- mode: C; -*-

uniform sampler2D tex_cloud;

varying vec2 texcoord;
varying vec3 lightdir;
varying vec3 lightcolor;
varying vec3 viewerdir; /* we need it for the rim lighting/blend */
varying float horizon_alpha;

const float density = 0.25; /* 0.0 = full density */
const float density_rcp = 1.33333; /* 1/(1-density) */

/* for clouds, g variable defaulted to 0.8 in singlescatter() but can be made
 * to vary later, weather system perhaps */
const float thick = 0.6; /* cloud thickness */
const float albedo = 0.1; /* cloud albedo */

/* henyey-greenstein phase function */
/* we defaulted to 0.8, backscattering, so we wouldn't need phase change, but
 * having it allow us some possibilities for scattering variation */
float phase( in vec3 wi, in vec3 wo, in float g)
{
    float costheta = dot( wi, wo );
    return (1.0 - g*g) / pow( 1.0 + g*g - 2.0 *g*costheta, 1.5);
}

/* single scattering approximation */
float singlescatter( in vec3 wi, in vec3 wo, in vec3 nn, in float g,
                        in float albedo, in float thickness )
{
    float win = abs( dot( wi, nn));
    float won = abs( dot( wo, nn));
    return albedo * phase( wo, wi, g) / (win + won) *
        (1.0 - exp( -(1.0/win + 1.0/won) * thickness));
}

/* main */
void main()
{
    /* viewer vector */
    vec3 E = normalize(viewerdir);

    /* get and normalize vector to light source */
    vec3 L = normalize(lightdir);

    const float d = 1.0/256.0; /* 1.0/256.0 */
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

    float ldotn = dot( L, N);
  
    /* vectors for single scattering&phase function */
    vec3 T = normalize( refract( E, N, 1.0 ) );
    vec3 T2 = normalize( refract( -L, N, 1.0 ) );

    /* first float, default 0.8 = g variable, negative g = forward scattering,
     * positive g = backscattering, we can add some variation for the weather
     * system later here */
    float scatter = singlescatter( T, T2, N, 0.8, albedo, thick);

    /* diffuse value */
    float c = clamp( ldotn, 0.0, 1.0);

    /* Getting a rim lighting effect, from -L+E vector */
    float rim = smoothstep(0.25, 1.0, 1.0-dot(normalize(-L+E), N));
    
    /* base cloud color, always multiplied by light color */
    const vec3 cloudcol = vec3(.86, .85, .89) * lightcolor;
    
    /* final mix - first component should have shadow color from sky color */
    vec3 final_color = mix( cloudcol * c + lightcolor * rim, lightcolor, 
         scatter);

    float b = (texture2D(tex_cloud, texcoord.xy).x
	       + texture2D(tex_cloud, texcoord.xy*2.5).x * 0.5
	       + texture2D(tex_cloud, texcoord.xy*5.0).x * 0.25
	       + texture2D(tex_cloud, texcoord.xy*9.5).x * 0.125
	       ) * (1.0/1.875);
    final_color = vec3(b, b, b);
    const float coverage = 0.5;
    b = max(b - 1.0 + coverage, 0.0) / coverage;
    alpha = b;

    /* We can improve this later, since clouds seem to behave with different
      color at horizon, we might use fog to do some tricks later, but for
      the time being, let's leave fog disabled */
    float fog_factor = clamp((gl_Fog.end - gl_FogFragCoord) * 
                        gl_Fog.scale, 0.0, 1.0);
    //fog_factor = exp2(-gl_Fog.density * gl_FogFragCoord /* * 1.4426940 */);

    /* final color, mix between fog and final color, but see above notes */
    gl_FragColor = vec4( mix(vec3(gl_Fog.color), final_color, fog_factor), alpha);
    //gl_FragColor = vec4( final_color, alpha);
}
