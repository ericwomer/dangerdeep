uniform sampler2D Pass0;
uniform sampler2D Pass1;
uniform sampler2D Pass2;
uniform sampler2D Pass3;
uniform vec4 bkgd;

void main(void)
{
    vec4 t0 = texture2D(Pass0, gl_TexCoord[0].st) * 0.75;
    vec4 t1 = texture2D(Pass1, gl_TexCoord[0].st) * 0.5;
    vec4 t2 = texture2D(Pass2, gl_TexCoord[0].st) * 0.5;
    vec4 t3 = texture2D(Pass3, gl_TexCoord[0].st) * 0.25;

    gl_FragColor = t0 + t1 + t2 + t3 + bkgd;
}
