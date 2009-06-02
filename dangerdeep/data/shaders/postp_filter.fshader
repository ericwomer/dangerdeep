// bloom // Philip Rideout

uniform sampler2D source;
uniform vec3 coefficients;
uniform float offsetx;
uniform float offsety;

void main(void)
{
    vec4 c;
    vec2 tc = gl_TexCoord[0].st;
    vec2 offset = vec2(offsetx, offsety);

    c  = coefficients.x * texture2D(source, tc - offset);
    c += coefficients.y * texture2D(source, tc);
    c += coefficients.z * texture2D(source, tc + offset);

    gl_FragColor = c;
}
