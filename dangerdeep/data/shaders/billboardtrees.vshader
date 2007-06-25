// -*- mode: C; -*-
#version 110

/* input:
   gl_Vertex
   gl_Texcoord0
   gl_Normal
*/

varying vec2 texcoord0;
varying float diffcol;
attribute float treesize;
uniform vec2 billboarddir;
/*
uniform float windmovement;
const float sf = 2.0*3.14159/64.0;
const float windmovescal = 0.5;
*/

void main()
{
	// fetch texcoord directly
	texcoord0 = gl_MultiTexCoord0.xy;

	// move tree tops in sine waves according to pos
	// we distuingish tree bottom from top vertices by their y texcoord
/*
	float f = (gl_Vertex.x + windmovement) * sf;
	vec2 windmove = vec2(sin(sf), cos(sf)) * (windmovescal * (1.0 - texcoord0.y));
*/

	// compute vertex position
	vec2 addpos = billboarddir * treesize /*+ windmove*/;
	vec4 vpos = gl_Vertex;
	vpos.xy += addpos;

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);

	// compute light brightness
	diffcol = dot(lightdir_obj, gl_Normal) * 0.75 + 0.25;

	// finally compute position
	gl_Position = gl_ModelViewProjectionMatrix * vpos;

	// set fog coordinate
	gl_FogFragCoord = gl_Position.z;
}
