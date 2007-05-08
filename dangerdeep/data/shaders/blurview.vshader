// -*- mode: C; -*-
#version 110

varying vec2 texc_0;
varying vec2 texc_1;

uniform vec3 blur_texc_offset;

void main()
{
	texc_0 = gl_MultiTexCoord0.xy;
	texc_1 = vec2(gl_MultiTexCoord0.x, gl_MultiTexCoord0.y * 0.5 + blur_texc_offset.x);
	gl_Position = ftransform();
}
