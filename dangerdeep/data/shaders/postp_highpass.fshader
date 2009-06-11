uniform sampler2D source;

const float thresh = 1.0;

void main(void)
{
	vec4 color = texture2D(source, gl_TexCoord[0].st);

	color.x = color.x > thresh ? 1.0 : 0.0;
	color.y = color.y > thresh ? 1.0 : 0.0;
	color.z = color.z > thresh ? 1.0 : 0.0;
	
	gl_FragColor = color;
}
