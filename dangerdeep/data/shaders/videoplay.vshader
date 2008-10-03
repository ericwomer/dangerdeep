// -*- mode: C; -*-

/* input:
   gl_Vertex
   gl_Texcoord0
*/

varying vec2 texcoord0;

void main()
{
	// fetch texcoord directly
	texcoord0 = gl_MultiTexCoord0.xy;

	gl_Position = ftransform();
}
