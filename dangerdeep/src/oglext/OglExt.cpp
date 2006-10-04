// OglExt.cpp                                                Copyright (C) 2006 thomas jansen (jansen@caesar.de)
//                                                                     (C) 2006 research center caesar
//
// This file is part of OglExt, a free OpenGL extension library.
//
// This program is free software; you can redistribute it and/or modify it under the terms  of  the  GNU  Lesser
// General Public License as published by the Free Software Foundation; either version 2.1 of  the  License,  or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;  without  even  the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser  General  Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with  this  library;  if  not,
// write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// This file was automatically generated on January 10, 2006, 6:46 pm

#if defined(_WIN32)
	#define	GLAPI __declspec(dllexport)
#else
	#define	GLAPI
#endif


#include	"OglExt.h"
#include	"Macros.h"

#include	"RenderingContext.hpp"
#include	"RCHashArray.hpp"


// ---[ GLOBAL VARIABLES ]--------------------------------------------------------------------------------------

CRCHashArray g_RCHashArray;


// =============================================================================================================
// ===                                A D D I T I O N A L   F U N C T I O N S                                ===
// =============================================================================================================

//!	Is list of extensions supported?

GLboolean glexExtensionsSupported(char const * szExtensions)
{
	// 1: get the current rendering context...

	CRenderingContext * pContext = g_RCHashArray.PrepareRenderingContext();
	if(!pContext) {

		return GL_FALSE;
	}

	// 2: call the context specific method...

	return pContext->IsExtensionSupported(szExtensions) ? GL_TRUE : GL_FALSE;
}


//!	Return the supported OpenGL version.

GLuint glexGetVersion()
{
	// 1: get the current rendering context...

	CRenderingContext * pContext = g_RCHashArray.PrepareRenderingContext();
	if(!pContext) {

		return 0;
	}

	// 2: call the context specific method...

	return (GLuint) pContext->GetVersion();
}


// =============================================================================================================
// ===                                            V E R S I O N                                              ===
// =============================================================================================================

// ---[ GL_VERSION_1_2 ]----------------------------------------------------------------------------------------

#ifdef GL_VERSION_1_2_OGLEXT

   GLvoid            WRAPPER04V(BlendColor, GLclampf, GLclampf, GLclampf, GLclampf)
   GLvoid            WRAPPER01V(BlendEquation, GLenum)
   GLvoid            WRAPPER06V(ColorSubTable, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER06V(ColorTable, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER03V(ColorTableParameterfv, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(ColorTableParameteriv, GLenum, GLenum, GLint const *)
   GLvoid            WRAPPER06V(ConvolutionFilter1D, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER07V(ConvolutionFilter2D, GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER03V(ConvolutionParameterf, GLenum, GLenum, GLfloat)
   GLvoid            WRAPPER03V(ConvolutionParameterfv, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(ConvolutionParameteri, GLenum, GLenum, GLint)
   GLvoid            WRAPPER03V(ConvolutionParameteriv, GLenum, GLenum, GLint const *)
   GLvoid            WRAPPER05V(CopyColorSubTable, GLenum, GLsizei, GLint, GLint, GLsizei)
   GLvoid            WRAPPER05V(CopyColorTable, GLenum, GLenum, GLint, GLint, GLsizei)
   GLvoid            WRAPPER05V(CopyConvolutionFilter1D, GLenum, GLenum, GLint, GLint, GLsizei)
   GLvoid            WRAPPER06V(CopyConvolutionFilter2D, GLenum, GLenum, GLint, GLint, GLsizei, GLsizei)
   GLvoid            WRAPPER09V(CopyTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)
   GLvoid            WRAPPER06V(DrawRangeElements, GLenum, GLuint, GLuint, GLsizei, GLenum, GLvoid const *)
   GLvoid            WRAPPER04V(GetColorTable, GLenum, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetColorTableParameterfv, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetColorTableParameteriv, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER04V(GetConvolutionFilter, GLenum, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetConvolutionParameterfv, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetConvolutionParameteriv, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER05V(GetHistogram, GLenum, GLboolean, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetHistogramParameterfv, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetHistogramParameteriv, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER05V(GetMinmax, GLenum, GLboolean, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetMinmaxParameterfv, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetMinmaxParameteriv, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER06V(GetSeparableFilter, GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *)
   GLvoid            WRAPPER04V(Histogram, GLenum, GLsizei, GLenum, GLboolean)
   GLvoid            WRAPPER03V(Minmax, GLenum, GLenum, GLboolean)
   GLvoid            WRAPPER01V(ResetHistogram, GLenum)
   GLvoid            WRAPPER01V(ResetMinmax, GLenum)
   GLvoid            WRAPPER08V(SeparableFilter2D, GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *, GLvoid const *)
   GLvoid            WRAPPER10V(TexImage3D, GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER11V(TexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)

#endif /* GL_VERSION_1_2_OGLEXT */

// ---[ GL_VERSION_1_3 ]----------------------------------------------------------------------------------------

#ifdef GL_VERSION_1_3_OGLEXT

   GLvoid            WRAPPER01V(ActiveTexture, GLenum)
   GLvoid            WRAPPER01V(ClientActiveTexture, GLenum)
   GLvoid            WRAPPER07V(CompressedTexImage1D, GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, GLvoid const *)
   GLvoid            WRAPPER08V(CompressedTexImage2D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *)
   GLvoid            WRAPPER09V(CompressedTexImage3D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *)
   GLvoid            WRAPPER07V(CompressedTexSubImage1D, GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER09V(CompressedTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER11V(CompressedTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER03V(GetCompressedTexImage, GLenum, GLint, GLvoid *)
   GLvoid            WRAPPER01V(LoadTransposeMatrixd, GLdouble const *)
   GLvoid            WRAPPER01V(LoadTransposeMatrixf, GLfloat const *)
   GLvoid            WRAPPER02V(MultiTexCoord1d, GLenum, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord1dv, GLenum, GLdouble const *)
   GLvoid            WRAPPER02V(MultiTexCoord1f, GLenum, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord1fv, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(MultiTexCoord1i, GLenum, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord1iv, GLenum, GLint const *)
   GLvoid            WRAPPER02V(MultiTexCoord1s, GLenum, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord1sv, GLenum, GLshort const *)
   GLvoid            WRAPPER03V(MultiTexCoord2d, GLenum, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord2dv, GLenum, GLdouble const *)
   GLvoid            WRAPPER03V(MultiTexCoord2f, GLenum, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord2fv, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(MultiTexCoord2i, GLenum, GLint, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord2iv, GLenum, GLint const *)
   GLvoid            WRAPPER03V(MultiTexCoord2s, GLenum, GLshort, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord2sv, GLenum, GLshort const *)
   GLvoid            WRAPPER04V(MultiTexCoord3d, GLenum, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord3dv, GLenum, GLdouble const *)
   GLvoid            WRAPPER04V(MultiTexCoord3f, GLenum, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord3fv, GLenum, GLfloat const *)
   GLvoid            WRAPPER04V(MultiTexCoord3i, GLenum, GLint, GLint, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord3iv, GLenum, GLint const *)
   GLvoid            WRAPPER04V(MultiTexCoord3s, GLenum, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord3sv, GLenum, GLshort const *)
   GLvoid            WRAPPER05V(MultiTexCoord4d, GLenum, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord4dv, GLenum, GLdouble const *)
   GLvoid            WRAPPER05V(MultiTexCoord4f, GLenum, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord4fv, GLenum, GLfloat const *)
   GLvoid            WRAPPER05V(MultiTexCoord4i, GLenum, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord4iv, GLenum, GLint const *)
   GLvoid            WRAPPER05V(MultiTexCoord4s, GLenum, GLshort, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord4sv, GLenum, GLshort const *)
   GLvoid            WRAPPER01V(MultTransposeMatrixd, GLdouble const *)
   GLvoid            WRAPPER01V(MultTransposeMatrixf, GLfloat const *)
   GLvoid            WRAPPER02V(SampleCoverage, GLclampf, GLboolean)

#endif /* GL_VERSION_1_3_OGLEXT */

// ---[ GL_VERSION_1_4 ]----------------------------------------------------------------------------------------

#ifdef GL_VERSION_1_4_OGLEXT

   GLvoid            WRAPPER04V(BlendFuncSeparate, GLenum, GLenum, GLenum, GLenum)
   GLvoid            WRAPPER01V(FogCoordd, GLdouble)
   GLvoid            WRAPPER01V(FogCoorddv, GLdouble const *)
   GLvoid            WRAPPER01V(FogCoordf, GLfloat)
   GLvoid            WRAPPER01V(FogCoordfv, GLfloat const *)
   GLvoid            WRAPPER03V(FogCoordPointer, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER04V(MultiDrawArrays, GLenum, GLint *, GLsizei *, GLsizei)
   GLvoid            WRAPPER05V(MultiDrawElements, GLenum, GLsizei const *, GLenum, GLvoid const * *, GLsizei)
   GLvoid            WRAPPER02V(PointParameterf, GLenum, GLfloat)
   GLvoid            WRAPPER02V(PointParameterfv, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(PointParameteri, GLenum, GLint)
   GLvoid            WRAPPER02V(PointParameteriv, GLenum, GLint const *)
   GLvoid            WRAPPER03V(SecondaryColor3b, GLbyte, GLbyte, GLbyte)
   GLvoid            WRAPPER01V(SecondaryColor3bv, GLbyte const *)
   GLvoid            WRAPPER03V(SecondaryColor3d, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(SecondaryColor3dv, GLdouble const *)
   GLvoid            WRAPPER03V(SecondaryColor3f, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(SecondaryColor3fv, GLfloat const *)
   GLvoid            WRAPPER03V(SecondaryColor3i, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(SecondaryColor3iv, GLint const *)
   GLvoid            WRAPPER03V(SecondaryColor3s, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(SecondaryColor3sv, GLshort const *)
   GLvoid            WRAPPER03V(SecondaryColor3ub, GLubyte, GLubyte, GLubyte)
   GLvoid            WRAPPER01V(SecondaryColor3ubv, GLubyte const *)
   GLvoid            WRAPPER03V(SecondaryColor3ui, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER01V(SecondaryColor3uiv, GLuint const *)
   GLvoid            WRAPPER03V(SecondaryColor3us, GLushort, GLushort, GLushort)
   GLvoid            WRAPPER01V(SecondaryColor3usv, GLushort const *)
   GLvoid            WRAPPER04V(SecondaryColorPointer, GLint, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER02V(WindowPos2d, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(WindowPos2dv, GLdouble const *)
   GLvoid            WRAPPER02V(WindowPos2f, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(WindowPos2fv, GLfloat const *)
   GLvoid            WRAPPER02V(WindowPos2i, GLint, GLint)
   GLvoid            WRAPPER01V(WindowPos2iv, GLint const *)
   GLvoid            WRAPPER02V(WindowPos2s, GLshort, GLshort)
   GLvoid            WRAPPER01V(WindowPos2sv, GLshort const *)
   GLvoid            WRAPPER03V(WindowPos3d, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(WindowPos3dv, GLdouble const *)
   GLvoid            WRAPPER03V(WindowPos3f, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(WindowPos3fv, GLfloat const *)
   GLvoid            WRAPPER03V(WindowPos3i, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(WindowPos3iv, GLint const *)
   GLvoid            WRAPPER03V(WindowPos3s, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(WindowPos3sv, GLshort const *)

#endif /* GL_VERSION_1_4_OGLEXT */

// ---[ GL_VERSION_1_5 ]----------------------------------------------------------------------------------------

#ifdef GL_VERSION_1_5_OGLEXT

   GLvoid            WRAPPER02V(BeginQuery, GLenum, GLuint)
   GLvoid            WRAPPER02V(BindBuffer, GLenum, GLuint)
   GLvoid            WRAPPER04V(BufferData, GLenum, GLsizeiptr, GLvoid const *, GLenum)
   GLvoid            WRAPPER04V(BufferSubData, GLenum, GLintptr, GLsizeiptr, GLvoid const *)
   GLvoid            WRAPPER02V(DeleteBuffers, GLsizei, GLuint const *)
   GLvoid            WRAPPER02V(DeleteQueries, GLsizei, GLuint const *)
   GLvoid            WRAPPER01V(EndQuery, GLenum)
   GLvoid            WRAPPER02V(GenBuffers, GLsizei, GLuint *)
   GLvoid            WRAPPER02V(GenQueries, GLsizei, GLuint *)
   GLvoid            WRAPPER03V(GetBufferParameteriv, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetBufferPointerv, GLenum, GLenum, GLvoid * *)
   GLvoid            WRAPPER04V(GetBufferSubData, GLenum, GLintptr, GLsizeiptr, GLvoid *)
   GLvoid            WRAPPER03V(GetQueryiv, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetQueryObjectiv, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetQueryObjectuiv, GLuint, GLenum, GLuint *)
   GLboolean         WRAPPER01R(IsBuffer, GLuint)
   GLboolean         WRAPPER01R(IsQuery, GLuint)
   GLvoid *          WRAPPER02R(MapBuffer, GLenum, GLenum)
   GLboolean         WRAPPER01R(UnmapBuffer, GLenum)

#endif /* GL_VERSION_1_5_OGLEXT */

// ---[ GL_VERSION_2_0 ]----------------------------------------------------------------------------------------

#ifdef GL_VERSION_2_0_OGLEXT

   GLvoid            WRAPPER02V(AttachShader, GLuint, GLuint)
   GLvoid            WRAPPER03V(BindAttribLocation, GLuint, GLuint, GLchar const *)
   GLvoid            WRAPPER02V(BlendEquationSeparate, GLenum, GLenum)
   GLvoid            WRAPPER01V(CompileShader, GLuint)
   GLuint            WRAPPER00R(CreateProgram)
   GLuint            WRAPPER01R(CreateShader, GLenum)
   GLvoid            WRAPPER01V(DeleteProgram, GLuint)
   GLvoid            WRAPPER01V(DeleteShader, GLuint)
   GLvoid            WRAPPER02V(DetachShader, GLuint, GLuint)
   GLvoid            WRAPPER01V(DisableVertexAttribArray, GLuint)
   GLvoid            WRAPPER02V(DrawBuffers, GLsizei, GLenum const *)
   GLvoid            WRAPPER01V(EnableVertexAttribArray, GLuint)
   GLvoid            WRAPPER07V(GetActiveAttrib, GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *)
   GLvoid            WRAPPER07V(GetActiveUniform, GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *)
   GLvoid            WRAPPER04V(GetAttachedShaders, GLuint, GLsizei, GLsizei *, GLuint *)
   GLint             WRAPPER02R(GetAttribLocation, GLuint, GLchar const *)
   GLvoid            WRAPPER04V(GetProgramInfoLog, GLuint, GLsizei, GLsizei *, GLchar *)
   GLvoid            WRAPPER03V(GetProgramiv, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER04V(GetShaderInfoLog, GLuint, GLsizei, GLsizei *, GLchar *)
   GLvoid            WRAPPER03V(GetShaderiv, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER04V(GetShaderSource, GLuint, GLsizei, GLsizei *, GLchar *)
   GLvoid            WRAPPER03V(GetUniformfv, GLuint, GLint, GLfloat *)
   GLvoid            WRAPPER03V(GetUniformiv, GLuint, GLint, GLint *)
   GLint             WRAPPER02R(GetUniformLocation, GLuint, GLchar const *)
   GLvoid            WRAPPER03V(GetVertexAttribdv, GLuint, GLenum, GLdouble *)
   GLvoid            WRAPPER03V(GetVertexAttribfv, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetVertexAttribiv, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetVertexAttribPointerv, GLuint, GLenum, GLvoid * *)
   GLboolean         WRAPPER01R(IsProgram, GLuint)
   GLboolean         WRAPPER01R(IsShader, GLuint)
   GLvoid            WRAPPER01V(LinkProgram, GLuint)
   GLvoid            WRAPPER04V(ShaderSource, GLuint, GLsizei, GLchar const * *, GLint const *)
   GLvoid            WRAPPER04V(StencilFuncSeparate, GLenum, GLenum, GLint, GLuint)
   GLvoid            WRAPPER02V(StencilMaskSeparate, GLenum, GLuint)
   GLvoid            WRAPPER04V(StencilOpSeparate, GLenum, GLenum, GLenum, GLenum)
   GLvoid            WRAPPER02V(Uniform1f, GLint, GLfloat)
   GLvoid            WRAPPER03V(Uniform1fv, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER02V(Uniform1i, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform1iv, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER03V(Uniform2f, GLint, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(Uniform2fv, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER03V(Uniform2i, GLint, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform2iv, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER04V(Uniform3f, GLint, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(Uniform3fv, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER04V(Uniform3i, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform3iv, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER05V(Uniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(Uniform4fv, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER05V(Uniform4i, GLint, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform4iv, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER04V(UniformMatrix2fv, GLint, GLsizei, GLboolean, GLfloat const *)
   GLvoid            WRAPPER04V(UniformMatrix3fv, GLint, GLsizei, GLboolean, GLfloat const *)
   GLvoid            WRAPPER04V(UniformMatrix4fv, GLint, GLsizei, GLboolean, GLfloat const *)
   GLvoid            WRAPPER01V(UseProgram, GLuint)
   GLvoid            WRAPPER01V(ValidateProgram, GLuint)
   GLvoid            WRAPPER02V(VertexAttrib1d, GLuint, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib1dv, GLuint, GLdouble const *)
   GLvoid            WRAPPER02V(VertexAttrib1f, GLuint, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib1fv, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(VertexAttrib1s, GLuint, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib1sv, GLuint, GLshort const *)
   GLvoid            WRAPPER03V(VertexAttrib2d, GLuint, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib2dv, GLuint, GLdouble const *)
   GLvoid            WRAPPER03V(VertexAttrib2f, GLuint, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib2fv, GLuint, GLfloat const *)
   GLvoid            WRAPPER03V(VertexAttrib2s, GLuint, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib2sv, GLuint, GLshort const *)
   GLvoid            WRAPPER04V(VertexAttrib3d, GLuint, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib3dv, GLuint, GLdouble const *)
   GLvoid            WRAPPER04V(VertexAttrib3f, GLuint, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib3fv, GLuint, GLfloat const *)
   GLvoid            WRAPPER04V(VertexAttrib3s, GLuint, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib3sv, GLuint, GLshort const *)
   GLvoid            WRAPPER02V(VertexAttrib4bv, GLuint, GLbyte const *)
   GLvoid            WRAPPER05V(VertexAttrib4d, GLuint, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib4dv, GLuint, GLdouble const *)
   GLvoid            WRAPPER05V(VertexAttrib4f, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib4fv, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(VertexAttrib4iv, GLuint, GLint const *)
   GLvoid            WRAPPER02V(VertexAttrib4Nbv, GLuint, GLbyte const *)
   GLvoid            WRAPPER02V(VertexAttrib4Niv, GLuint, GLint const *)
   GLvoid            WRAPPER02V(VertexAttrib4Nsv, GLuint, GLshort const *)
   GLvoid            WRAPPER05V(VertexAttrib4Nub, GLuint, GLubyte, GLubyte, GLubyte, GLubyte)
   GLvoid            WRAPPER02V(VertexAttrib4Nubv, GLuint, GLubyte const *)
   GLvoid            WRAPPER02V(VertexAttrib4Nuiv, GLuint, GLuint const *)
   GLvoid            WRAPPER02V(VertexAttrib4Nusv, GLuint, GLushort const *)
   GLvoid            WRAPPER05V(VertexAttrib4s, GLuint, GLshort, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib4sv, GLuint, GLshort const *)
   GLvoid            WRAPPER02V(VertexAttrib4ubv, GLuint, GLubyte const *)
   GLvoid            WRAPPER02V(VertexAttrib4uiv, GLuint, GLuint const *)
   GLvoid            WRAPPER02V(VertexAttrib4usv, GLuint, GLushort const *)
   GLvoid            WRAPPER06V(VertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, GLvoid const *)

#endif /* GL_VERSION_2_0_OGLEXT */

// =============================================================================================================
// ===                                          E X T E N S I O N                                            ===
// =============================================================================================================

// ---[ GL_3DFX_tbuffer ]---------------------------------------------------------------------------------------

#ifdef GL_3DFX_tbuffer_OGLEXT

   GLvoid            WRAPPER01V(TbufferMask3DFX, GLuint)

#endif /* GL_3DFX_tbuffer_OGLEXT */

// ---[ GL_APPLE_element_array ]--------------------------------------------------------------------------------

#ifdef GL_APPLE_element_array_OGLEXT

   GLvoid            WRAPPER03V(DrawElementArrayAPPLE, GLenum, GLint, GLsizei)
   GLvoid            WRAPPER05V(DrawRangeElementArrayAPPLE, GLenum, GLuint, GLuint, GLint, GLsizei)
   GLvoid            WRAPPER02V(ElementPointerAPPLE, GLenum, GLvoid const *)
   GLvoid            WRAPPER04V(MultiDrawElementArrayAPPLE, GLenum, GLint const *, GLsizei const *, GLsizei)
   GLvoid            WRAPPER06V(MultiDrawRangeElementArrayAPPLE, GLenum, GLuint, GLuint, GLint const *, GLsizei const *, GLsizei)

#endif /* GL_APPLE_element_array_OGLEXT */

// ---[ GL_APPLE_fence ]----------------------------------------------------------------------------------------

#ifdef GL_APPLE_fence_OGLEXT

   GLvoid            WRAPPER02V(DeleteFencesAPPLE, GLsizei, GLuint const *)
   GLvoid            WRAPPER01V(FinishFenceAPPLE, GLuint)
   GLvoid            WRAPPER02V(FinishObjectAPPLE, GLenum, GLint)
   GLvoid            WRAPPER02V(GenFencesAPPLE, GLsizei, GLuint *)
   GLboolean         WRAPPER01R(IsFenceAPPLE, GLuint)
   GLvoid            WRAPPER01V(SetFenceAPPLE, GLuint)
   GLboolean         WRAPPER01R(TestFenceAPPLE, GLuint)
   GLboolean         WRAPPER02R(TestObjectAPPLE, GLenum, GLuint)

#endif /* GL_APPLE_fence_OGLEXT */

// ---[ GL_APPLE_vertex_array_object ]--------------------------------------------------------------------------

#ifdef GL_APPLE_vertex_array_object_OGLEXT

   GLvoid            WRAPPER01V(BindVertexArrayAPPLE, GLuint)
   GLvoid            WRAPPER02V(DeleteVertexArraysAPPLE, GLsizei, GLuint const *)
   GLvoid            WRAPPER02V(GenVertexArraysAPPLE, GLsizei, GLuint const *)
   GLboolean         WRAPPER01R(IsVertexArrayAPPLE, GLuint)

#endif /* GL_APPLE_vertex_array_object_OGLEXT */

// ---[ GL_APPLE_vertex_array_range ]---------------------------------------------------------------------------

#ifdef GL_APPLE_vertex_array_range_OGLEXT

   GLvoid            WRAPPER02V(FlushVertexArrayRangeAPPLE, GLsizei, GLvoid *)
   GLvoid            WRAPPER02V(VertexArrayParameteriAPPLE, GLenum, GLint)
   GLvoid            WRAPPER02V(VertexArrayRangeAPPLE, GLsizei, GLvoid *)

#endif /* GL_APPLE_vertex_array_range_OGLEXT */

// ---[ GL_ARB_color_buffer_float ]-----------------------------------------------------------------------------

#ifdef GL_ARB_color_buffer_float_OGLEXT

   GLvoid            WRAPPER02V(ClampColorARB, GLenum, GLenum)

#endif /* GL_ARB_color_buffer_float_OGLEXT */

// ---[ GL_ARB_draw_buffers ]-----------------------------------------------------------------------------------

#ifdef GL_ARB_draw_buffers_OGLEXT

   GLvoid            WRAPPER02V(DrawBuffersARB, GLsizei, GLenum const *)

#endif /* GL_ARB_draw_buffers_OGLEXT */

// ---[ GL_ARB_matrix_palette ]---------------------------------------------------------------------------------

#ifdef GL_ARB_matrix_palette_OGLEXT

   GLvoid            WRAPPER01V(CurrentPaletteMatrixARB, GLint)
   GLvoid            WRAPPER04V(MatrixIndexPointerARB, GLint, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER02V(MatrixIndexubvARB, GLint, GLubyte const *)
   GLvoid            WRAPPER02V(MatrixIndexuivARB, GLint, GLuint const *)
   GLvoid            WRAPPER02V(MatrixIndexusvARB, GLint, GLushort const *)

#endif /* GL_ARB_matrix_palette_OGLEXT */

// ---[ GL_ARB_multisample ]------------------------------------------------------------------------------------

#ifdef GL_ARB_multisample_OGLEXT

   GLvoid            WRAPPER02V(SampleCoverageARB, GLclampf, GLboolean)

#endif /* GL_ARB_multisample_OGLEXT */

// ---[ GL_ARB_multitexture ]-----------------------------------------------------------------------------------

#ifdef GL_ARB_multitexture_OGLEXT

   GLvoid            WRAPPER01V(ActiveTextureARB, GLenum)
   GLvoid            WRAPPER01V(ClientActiveTextureARB, GLenum)
   GLvoid            WRAPPER02V(MultiTexCoord1dARB, GLenum, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord1dvARB, GLenum, GLdouble const *)
   GLvoid            WRAPPER02V(MultiTexCoord1fARB, GLenum, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord1fvARB, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(MultiTexCoord1iARB, GLenum, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord1ivARB, GLenum, GLint const *)
   GLvoid            WRAPPER02V(MultiTexCoord1sARB, GLenum, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord1svARB, GLenum, GLshort const *)
   GLvoid            WRAPPER03V(MultiTexCoord2dARB, GLenum, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord2dvARB, GLenum, GLdouble const *)
   GLvoid            WRAPPER03V(MultiTexCoord2fARB, GLenum, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord2fvARB, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(MultiTexCoord2iARB, GLenum, GLint, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord2ivARB, GLenum, GLint const *)
   GLvoid            WRAPPER03V(MultiTexCoord2sARB, GLenum, GLshort, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord2svARB, GLenum, GLshort const *)
   GLvoid            WRAPPER04V(MultiTexCoord3dARB, GLenum, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord3dvARB, GLenum, GLdouble const *)
   GLvoid            WRAPPER04V(MultiTexCoord3fARB, GLenum, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord3fvARB, GLenum, GLfloat const *)
   GLvoid            WRAPPER04V(MultiTexCoord3iARB, GLenum, GLint, GLint, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord3ivARB, GLenum, GLint const *)
   GLvoid            WRAPPER04V(MultiTexCoord3sARB, GLenum, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord3svARB, GLenum, GLshort const *)
   GLvoid            WRAPPER05V(MultiTexCoord4dARB, GLenum, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(MultiTexCoord4dvARB, GLenum, GLdouble const *)
   GLvoid            WRAPPER05V(MultiTexCoord4fARB, GLenum, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(MultiTexCoord4fvARB, GLenum, GLfloat const *)
   GLvoid            WRAPPER05V(MultiTexCoord4iARB, GLenum, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER02V(MultiTexCoord4ivARB, GLenum, GLint const *)
   GLvoid            WRAPPER05V(MultiTexCoord4sARB, GLenum, GLshort, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(MultiTexCoord4svARB, GLenum, GLshort const *)

#endif /* GL_ARB_multitexture_OGLEXT */

// ---[ GL_ARB_occlusion_query ]--------------------------------------------------------------------------------

#ifdef GL_ARB_occlusion_query_OGLEXT

   GLvoid            WRAPPER02V(BeginQueryARB, GLenum, GLuint)
   GLvoid            WRAPPER02V(DeleteQueriesARB, GLsizei, GLuint const *)
   GLvoid            WRAPPER01V(EndQueryARB, GLenum)
   GLvoid            WRAPPER02V(GenQueriesARB, GLsizei, GLuint *)
   GLvoid            WRAPPER03V(GetQueryivARB, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetQueryObjectivARB, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetQueryObjectuivARB, GLuint, GLenum, GLuint *)
   GLboolean         WRAPPER01R(IsQueryARB, GLuint)

#endif /* GL_ARB_occlusion_query_OGLEXT */

// ---[ GL_ARB_point_parameters ]-------------------------------------------------------------------------------

#ifdef GL_ARB_point_parameters_OGLEXT

   GLvoid            WRAPPER02V(PointParameterfARB, GLenum, GLfloat)
   GLvoid            WRAPPER02V(PointParameterfvARB, GLenum, GLfloat const *)

#endif /* GL_ARB_point_parameters_OGLEXT */

// ---[ GL_ARB_shader_objects ]---------------------------------------------------------------------------------

#ifdef GL_ARB_shader_objects_OGLEXT

   GLvoid            WRAPPER02V(AttachObjectARB, GLhandleARB, GLhandleARB)
   GLvoid            WRAPPER01V(CompileShaderARB, GLhandleARB)
   GLhandleARB       WRAPPER00R(CreateProgramObjectARB)
   GLhandleARB       WRAPPER01R(CreateShaderObjectARB, GLenum)
   GLvoid            WRAPPER01V(DeleteObjectARB, GLhandleARB)
   GLvoid            WRAPPER02V(DetachObjectARB, GLhandleARB, GLhandleARB)
   GLvoid            WRAPPER07V(GetActiveUniformARB, GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *)
   GLvoid            WRAPPER04V(GetAttachedObjectsARB, GLhandleARB, GLsizei, GLsizei *, GLhandleARB *)
   GLhandleARB       WRAPPER01R(GetHandleARB, GLenum)
   GLvoid            WRAPPER04V(GetInfoLogARB, GLhandleARB, GLsizei, GLsizei *, GLcharARB *)
   GLvoid            WRAPPER03V(GetObjectParameterfvARB, GLhandleARB, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetObjectParameterivARB, GLhandleARB, GLenum, GLint *)
   GLvoid            WRAPPER04V(GetShaderSourceARB, GLhandleARB, GLsizei, GLsizei *, GLcharARB *)
   GLvoid            WRAPPER03V(GetUniformfvARB, GLhandleARB, GLint, GLfloat *)
   GLvoid            WRAPPER03V(GetUniformivARB, GLhandleARB, GLint, GLint *)
   GLint             WRAPPER02R(GetUniformLocationARB, GLhandleARB, GLcharARB const *)
   GLvoid            WRAPPER01V(LinkProgramARB, GLhandleARB)
   GLvoid            WRAPPER04V(ShaderSourceARB, GLhandleARB, GLsizei, GLcharARB const * *, GLint const *)
   GLvoid            WRAPPER02V(Uniform1fARB, GLint, GLfloat)
   GLvoid            WRAPPER03V(Uniform1fvARB, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER02V(Uniform1iARB, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform1ivARB, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER03V(Uniform2fARB, GLint, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(Uniform2fvARB, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER03V(Uniform2iARB, GLint, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform2ivARB, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER04V(Uniform3fARB, GLint, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(Uniform3fvARB, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER04V(Uniform3iARB, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform3ivARB, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER05V(Uniform4fARB, GLint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(Uniform4fvARB, GLint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER05V(Uniform4iARB, GLint, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER03V(Uniform4ivARB, GLint, GLsizei, GLint const *)
   GLvoid            WRAPPER04V(UniformMatrix2fvARB, GLint, GLsizei, GLboolean, GLfloat const *)
   GLvoid            WRAPPER04V(UniformMatrix3fvARB, GLint, GLsizei, GLboolean, GLfloat const *)
   GLvoid            WRAPPER04V(UniformMatrix4fvARB, GLint, GLsizei, GLboolean, GLfloat const *)
   GLvoid            WRAPPER01V(UseProgramObjectARB, GLhandleARB)
   GLvoid            WRAPPER01V(ValidateProgramARB, GLhandleARB)

#endif /* GL_ARB_shader_objects_OGLEXT */

// ---[ GL_ARB_texture_compression ]----------------------------------------------------------------------------

#ifdef GL_ARB_texture_compression_OGLEXT

   GLvoid            WRAPPER07V(CompressedTexImage1DARB, GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, GLvoid const *)
   GLvoid            WRAPPER08V(CompressedTexImage2DARB, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *)
   GLvoid            WRAPPER09V(CompressedTexImage3DARB, GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *)
   GLvoid            WRAPPER07V(CompressedTexSubImage1DARB, GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER09V(CompressedTexSubImage2DARB, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER11V(CompressedTexSubImage3DARB, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER03V(GetCompressedTexImageARB, GLenum, GLint, GLvoid *)

#endif /* GL_ARB_texture_compression_OGLEXT */

// ---[ GL_ARB_transpose_matrix ]-------------------------------------------------------------------------------

#ifdef GL_ARB_transpose_matrix_OGLEXT

   GLvoid            WRAPPER01V(LoadTransposeMatrixdARB, GLdouble const *)
   GLvoid            WRAPPER01V(LoadTransposeMatrixfARB, GLfloat const *)
   GLvoid            WRAPPER01V(MultTransposeMatrixdARB, GLdouble const *)
   GLvoid            WRAPPER01V(MultTransposeMatrixfARB, GLfloat const *)

#endif /* GL_ARB_transpose_matrix_OGLEXT */

// ---[ GL_ARB_vertex_blend ]-----------------------------------------------------------------------------------

#ifdef GL_ARB_vertex_blend_OGLEXT

   GLvoid            WRAPPER01V(VertexBlendARB, GLint)
   GLvoid            WRAPPER02V(WeightbvARB, GLint, GLbyte const *)
   GLvoid            WRAPPER02V(WeightdvARB, GLint, GLdouble const *)
   GLvoid            WRAPPER02V(WeightfvARB, GLint, GLfloat const *)
   GLvoid            WRAPPER02V(WeightivARB, GLint, GLint const *)
   GLvoid            WRAPPER04V(WeightPointerARB, GLint, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER02V(WeightsvARB, GLint, GLshort const *)
   GLvoid            WRAPPER02V(WeightubvARB, GLint, GLubyte const *)
   GLvoid            WRAPPER02V(WeightuivARB, GLint, GLuint const *)
   GLvoid            WRAPPER02V(WeightusvARB, GLint, GLushort const *)

#endif /* GL_ARB_vertex_blend_OGLEXT */

// ---[ GL_ARB_vertex_buffer_object ]---------------------------------------------------------------------------

#ifdef GL_ARB_vertex_buffer_object_OGLEXT

   GLvoid            WRAPPER02V(BindBufferARB, GLenum, GLuint)
   GLvoid            WRAPPER04V(BufferDataARB, GLenum, GLsizeiptrARB, GLvoid const *, GLenum)
   GLvoid            WRAPPER04V(BufferSubDataARB, GLenum, GLintptrARB, GLsizeiptrARB, GLvoid const *)
   GLvoid            WRAPPER02V(DeleteBuffersARB, GLsizei, GLuint const *)
   GLvoid            WRAPPER02V(GenBuffersARB, GLsizei, GLuint *)
   GLvoid            WRAPPER03V(GetBufferParameterivARB, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetBufferPointervARB, GLenum, GLenum, GLvoid * *)
   GLvoid            WRAPPER04V(GetBufferSubDataARB, GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *)
   GLboolean         WRAPPER01R(IsBufferARB, GLuint)
   GLvoid *          WRAPPER02R(MapBufferARB, GLenum, GLenum)
   GLboolean         WRAPPER01R(UnmapBufferARB, GLenum)

#endif /* GL_ARB_vertex_buffer_object_OGLEXT */

// ---[ GL_ARB_vertex_program ]---------------------------------------------------------------------------------

#ifdef GL_ARB_vertex_program_OGLEXT

   GLvoid            WRAPPER02V(BindProgramARB, GLenum, GLuint)
   GLvoid            WRAPPER02V(DeleteProgramsARB, GLsizei, GLuint const *)
   GLvoid            WRAPPER01V(DisableVertexAttribArrayARB, GLuint)
   GLvoid            WRAPPER01V(EnableVertexAttribArrayARB, GLuint)
   GLvoid            WRAPPER02V(GenProgramsARB, GLsizei, GLuint *)
   GLvoid            WRAPPER03V(GetProgramEnvParameterdvARB, GLenum, GLuint, GLdouble *)
   GLvoid            WRAPPER03V(GetProgramEnvParameterfvARB, GLenum, GLuint, GLfloat *)
   GLvoid            WRAPPER03V(GetProgramivARB, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetProgramLocalParameterdvARB, GLenum, GLuint, GLdouble *)
   GLvoid            WRAPPER03V(GetProgramLocalParameterfvARB, GLenum, GLuint, GLfloat *)
   GLvoid            WRAPPER03V(GetProgramStringARB, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetVertexAttribdvARB, GLuint, GLenum, GLdouble *)
   GLvoid            WRAPPER03V(GetVertexAttribfvARB, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetVertexAttribivARB, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetVertexAttribPointervARB, GLuint, GLenum, GLvoid * *)
   GLboolean         WRAPPER01R(IsProgramARB, GLuint)
   GLvoid            WRAPPER06V(ProgramEnvParameter4dARB, GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER03V(ProgramEnvParameter4dvARB, GLenum, GLuint, GLdouble const *)
   GLvoid            WRAPPER06V(ProgramEnvParameter4fARB, GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(ProgramEnvParameter4fvARB, GLenum, GLuint, GLfloat const *)
   GLvoid            WRAPPER06V(ProgramLocalParameter4dARB, GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER03V(ProgramLocalParameter4dvARB, GLenum, GLuint, GLdouble const *)
   GLvoid            WRAPPER06V(ProgramLocalParameter4fARB, GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(ProgramLocalParameter4fvARB, GLenum, GLuint, GLfloat const *)
   GLvoid            WRAPPER04V(ProgramStringARB, GLenum, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER02V(VertexAttrib1dARB, GLuint, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib1dvARB, GLuint, GLdouble const *)
   GLvoid            WRAPPER02V(VertexAttrib1fARB, GLuint, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib1fvARB, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(VertexAttrib1sARB, GLuint, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib1svARB, GLuint, GLshort const *)
   GLvoid            WRAPPER03V(VertexAttrib2dARB, GLuint, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib2dvARB, GLuint, GLdouble const *)
   GLvoid            WRAPPER03V(VertexAttrib2fARB, GLuint, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib2fvARB, GLuint, GLfloat const *)
   GLvoid            WRAPPER03V(VertexAttrib2sARB, GLuint, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib2svARB, GLuint, GLshort const *)
   GLvoid            WRAPPER04V(VertexAttrib3dARB, GLuint, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib3dvARB, GLuint, GLdouble const *)
   GLvoid            WRAPPER04V(VertexAttrib3fARB, GLuint, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib3fvARB, GLuint, GLfloat const *)
   GLvoid            WRAPPER04V(VertexAttrib3sARB, GLuint, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib3svARB, GLuint, GLshort const *)
   GLvoid            WRAPPER02V(VertexAttrib4bvARB, GLuint, GLbyte const *)
   GLvoid            WRAPPER05V(VertexAttrib4dARB, GLuint, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib4dvARB, GLuint, GLdouble const *)
   GLvoid            WRAPPER05V(VertexAttrib4fARB, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib4fvARB, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(VertexAttrib4ivARB, GLuint, GLint const *)
   GLvoid            WRAPPER02V(VertexAttrib4NbvARB, GLuint, GLbyte const *)
   GLvoid            WRAPPER02V(VertexAttrib4NivARB, GLuint, GLint const *)
   GLvoid            WRAPPER02V(VertexAttrib4NsvARB, GLuint, GLshort const *)
   GLvoid            WRAPPER05V(VertexAttrib4NubARB, GLuint, GLubyte, GLubyte, GLubyte, GLubyte)
   GLvoid            WRAPPER02V(VertexAttrib4NubvARB, GLuint, GLubyte const *)
   GLvoid            WRAPPER02V(VertexAttrib4NuivARB, GLuint, GLuint const *)
   GLvoid            WRAPPER02V(VertexAttrib4NusvARB, GLuint, GLushort const *)
   GLvoid            WRAPPER05V(VertexAttrib4sARB, GLuint, GLshort, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib4svARB, GLuint, GLshort const *)
   GLvoid            WRAPPER02V(VertexAttrib4ubvARB, GLuint, GLubyte const *)
   GLvoid            WRAPPER02V(VertexAttrib4uivARB, GLuint, GLuint const *)
   GLvoid            WRAPPER02V(VertexAttrib4usvARB, GLuint, GLushort const *)
   GLvoid            WRAPPER06V(VertexAttribPointerARB, GLuint, GLint, GLenum, GLboolean, GLsizei, GLvoid const *)

#endif /* GL_ARB_vertex_program_OGLEXT */

// ---[ GL_ARB_vertex_shader ]----------------------------------------------------------------------------------

#ifdef GL_ARB_vertex_shader_OGLEXT

   GLvoid            WRAPPER03V(BindAttribLocationARB, GLhandleARB, GLuint, GLcharARB const *)
   GLvoid            WRAPPER07V(GetActiveAttribARB, GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *)
   GLint             WRAPPER02R(GetAttribLocationARB, GLhandleARB, GLcharARB const *)

#endif /* GL_ARB_vertex_shader_OGLEXT */

// ---[ GL_ARB_window_pos ]-------------------------------------------------------------------------------------

#ifdef GL_ARB_window_pos_OGLEXT

   GLvoid            WRAPPER02V(WindowPos2dARB, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(WindowPos2dvARB, GLdouble const *)
   GLvoid            WRAPPER02V(WindowPos2fARB, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(WindowPos2fvARB, GLfloat const *)
   GLvoid            WRAPPER02V(WindowPos2iARB, GLint, GLint)
   GLvoid            WRAPPER01V(WindowPos2ivARB, GLint const *)
   GLvoid            WRAPPER02V(WindowPos2sARB, GLshort, GLshort)
   GLvoid            WRAPPER01V(WindowPos2svARB, GLshort const *)
   GLvoid            WRAPPER03V(WindowPos3dARB, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(WindowPos3dvARB, GLdouble const *)
   GLvoid            WRAPPER03V(WindowPos3fARB, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(WindowPos3fvARB, GLfloat const *)
   GLvoid            WRAPPER03V(WindowPos3iARB, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(WindowPos3ivARB, GLint const *)
   GLvoid            WRAPPER03V(WindowPos3sARB, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(WindowPos3svARB, GLshort const *)

#endif /* GL_ARB_window_pos_OGLEXT */

// ---[ GL_ATI_draw_buffers ]-----------------------------------------------------------------------------------

#ifdef GL_ATI_draw_buffers_OGLEXT

   GLvoid            WRAPPER02V(DrawBuffersATI, GLsizei, GLenum const *)

#endif /* GL_ATI_draw_buffers_OGLEXT */

// ---[ GL_ATI_element_array ]----------------------------------------------------------------------------------

#ifdef GL_ATI_element_array_OGLEXT

   GLvoid            WRAPPER02V(DrawElementArrayATI, GLenum, GLsizei)
   GLvoid            WRAPPER04V(DrawRangeElementArrayATI, GLenum, GLuint, GLuint, GLsizei)
   GLvoid            WRAPPER02V(ElementPointerATI, GLenum, GLvoid const *)

#endif /* GL_ATI_element_array_OGLEXT */

// ---[ GL_ATI_envmap_bumpmap ]---------------------------------------------------------------------------------

#ifdef GL_ATI_envmap_bumpmap_OGLEXT

   GLvoid            WRAPPER02V(GetTexBumpParameterfvATI, GLenum, GLfloat *)
   GLvoid            WRAPPER02V(GetTexBumpParameterivATI, GLenum, GLint *)
   GLvoid            WRAPPER02V(TexBumpParameterfvATI, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(TexBumpParameterivATI, GLenum, GLint const *)

#endif /* GL_ATI_envmap_bumpmap_OGLEXT */

// ---[ GL_ATI_fragment_shader ]--------------------------------------------------------------------------------

#ifdef GL_ATI_fragment_shader_OGLEXT

   GLvoid            WRAPPER06V(AlphaFragmentOp1ATI, GLenum, GLuint, GLuint, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER09V(AlphaFragmentOp2ATI, GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER12V(AlphaFragmentOp3ATI, GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER00V(BeginFragmentShaderATI)
   GLvoid            WRAPPER01V(BindFragmentShaderATI, GLuint)
   GLvoid            WRAPPER07V(ColorFragmentOp1ATI, GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER10V(ColorFragmentOp2ATI, GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER13V(ColorFragmentOp3ATI, GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER01V(DeleteFragmentShaderATI, GLuint)
   GLvoid            WRAPPER00V(EndFragmentShaderATI)
   GLuint            WRAPPER01R(GenFragmentShadersATI, GLuint)
   GLvoid            WRAPPER03V(PassTexCoordATI, GLuint, GLuint, GLenum)
   GLvoid            WRAPPER03V(SampleMapATI, GLuint, GLuint, GLenum)
   GLvoid            WRAPPER02V(SetFragmentShaderConstantATI, GLuint, GLfloat const *)

#endif /* GL_ATI_fragment_shader_OGLEXT */

// ---[ GL_ATI_map_object_buffer ]------------------------------------------------------------------------------

#ifdef GL_ATI_map_object_buffer_OGLEXT

   GLvoid *          WRAPPER01R(MapObjectBufferATI, GLuint)
   GLvoid            WRAPPER01V(UnmapObjectBufferATI, GLuint)

#endif /* GL_ATI_map_object_buffer_OGLEXT */

// ---[ GL_ATI_pn_triangles ]-----------------------------------------------------------------------------------

#ifdef GL_ATI_pn_triangles_OGLEXT

   GLvoid            WRAPPER02V(PNTrianglesfATI, GLenum, GLfloat)
   GLvoid            WRAPPER02V(PNTrianglesiATI, GLenum, GLint)

#endif /* GL_ATI_pn_triangles_OGLEXT */

// ---[ GL_ATI_separate_stencil ]-------------------------------------------------------------------------------

#ifdef GL_ATI_separate_stencil_OGLEXT

   GLvoid            WRAPPER04V(StencilFuncSeparateATI, GLenum, GLenum, GLint, GLuint)
   GLvoid            WRAPPER04V(StencilOpSeparateATI, GLenum, GLenum, GLenum, GLenum)

#endif /* GL_ATI_separate_stencil_OGLEXT */

// ---[ GL_ATI_vertex_array_object ]----------------------------------------------------------------------------

#ifdef GL_ATI_vertex_array_object_OGLEXT

   GLvoid            WRAPPER06V(ArrayObjectATI, GLenum, GLint, GLenum, GLsizei, GLuint, GLuint)
   GLvoid            WRAPPER01V(FreeObjectBufferATI, GLuint)
   GLvoid            WRAPPER03V(GetArrayObjectfvATI, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetArrayObjectivATI, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetObjectBufferfvATI, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetObjectBufferivATI, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetVariantArrayObjectfvATI, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetVariantArrayObjectivATI, GLuint, GLenum, GLint *)
   GLboolean         WRAPPER01R(IsObjectBufferATI, GLuint)
   GLuint            WRAPPER03R(NewObjectBufferATI, GLsizei, GLvoid const *, GLenum)
   GLvoid            WRAPPER05V(UpdateObjectBufferATI, GLuint, GLuint, GLsizei, GLvoid const *, GLenum)
   GLvoid            WRAPPER05V(VariantArrayObjectATI, GLuint, GLenum, GLsizei, GLuint, GLuint)

#endif /* GL_ATI_vertex_array_object_OGLEXT */

// ---[ GL_ATI_vertex_attrib_array_object ]---------------------------------------------------------------------

#ifdef GL_ATI_vertex_attrib_array_object_OGLEXT

   GLvoid            WRAPPER03V(GetVertexAttribArrayObjectfvATI, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetVertexAttribArrayObjectivATI, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER07V(VertexAttribArrayObjectATI, GLuint, GLint, GLenum, GLboolean, GLsizei, GLuint, GLuint)

#endif /* GL_ATI_vertex_attrib_array_object_OGLEXT */

// ---[ GL_ATI_vertex_streams ]---------------------------------------------------------------------------------

#ifdef GL_ATI_vertex_streams_OGLEXT

   GLvoid            WRAPPER01V(ClientActiveVertexStreamATI, GLenum)
   GLvoid            WRAPPER04V(NormalStream3bATI, GLenum, GLbyte, GLbyte, GLbyte)
   GLvoid            WRAPPER02V(NormalStream3bvATI, GLenum, GLbyte const *)
   GLvoid            WRAPPER04V(NormalStream3dATI, GLenum, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(NormalStream3dvATI, GLenum, GLdouble const *)
   GLvoid            WRAPPER04V(NormalStream3fATI, GLenum, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(NormalStream3fvATI, GLenum, GLfloat const *)
   GLvoid            WRAPPER04V(NormalStream3iATI, GLenum, GLint, GLint, GLint)
   GLvoid            WRAPPER02V(NormalStream3ivATI, GLenum, GLint const *)
   GLvoid            WRAPPER04V(NormalStream3sATI, GLenum, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(NormalStream3svATI, GLenum, GLshort const *)
   GLvoid            WRAPPER02V(VertexBlendEnvfATI, GLenum, GLfloat)
   GLvoid            WRAPPER02V(VertexBlendEnviATI, GLenum, GLint)
   GLvoid            WRAPPER02V(VertexStream1dATI, GLenum, GLdouble)
   GLvoid            WRAPPER02V(VertexStream1dvATI, GLenum, GLdouble const *)
   GLvoid            WRAPPER02V(VertexStream1fATI, GLenum, GLfloat)
   GLvoid            WRAPPER02V(VertexStream1fvATI, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(VertexStream1iATI, GLenum, GLint)
   GLvoid            WRAPPER02V(VertexStream1ivATI, GLenum, GLint const *)
   GLvoid            WRAPPER02V(VertexStream1sATI, GLenum, GLshort)
   GLvoid            WRAPPER02V(VertexStream1svATI, GLenum, GLshort const *)
   GLvoid            WRAPPER03V(VertexStream2dATI, GLenum, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexStream2dvATI, GLenum, GLdouble const *)
   GLvoid            WRAPPER03V(VertexStream2fATI, GLenum, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexStream2fvATI, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(VertexStream2iATI, GLenum, GLint, GLint)
   GLvoid            WRAPPER02V(VertexStream2ivATI, GLenum, GLint const *)
   GLvoid            WRAPPER03V(VertexStream2sATI, GLenum, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexStream2svATI, GLenum, GLshort const *)
   GLvoid            WRAPPER04V(VertexStream3dATI, GLenum, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexStream3dvATI, GLenum, GLdouble const *)
   GLvoid            WRAPPER04V(VertexStream3fATI, GLenum, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexStream3fvATI, GLenum, GLfloat const *)
   GLvoid            WRAPPER04V(VertexStream3iATI, GLenum, GLint, GLint, GLint)
   GLvoid            WRAPPER02V(VertexStream3ivATI, GLenum, GLint const *)
   GLvoid            WRAPPER04V(VertexStream3sATI, GLenum, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexStream3svATI, GLenum, GLshort const *)
   GLvoid            WRAPPER05V(VertexStream4dATI, GLenum, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexStream4dvATI, GLenum, GLdouble const *)
   GLvoid            WRAPPER05V(VertexStream4fATI, GLenum, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexStream4fvATI, GLenum, GLfloat const *)
   GLvoid            WRAPPER05V(VertexStream4iATI, GLenum, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER02V(VertexStream4ivATI, GLenum, GLint const *)
   GLvoid            WRAPPER05V(VertexStream4sATI, GLenum, GLshort, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexStream4svATI, GLenum, GLshort const *)

#endif /* GL_ATI_vertex_streams_OGLEXT */

// ---[ GL_EXT_blend_color ]------------------------------------------------------------------------------------

#ifdef GL_EXT_blend_color_OGLEXT

   GLvoid            WRAPPER04V(BlendColorEXT, GLclampf, GLclampf, GLclampf, GLclampf)

#endif /* GL_EXT_blend_color_OGLEXT */

// ---[ GL_EXT_blend_equation_separate ]------------------------------------------------------------------------

#ifdef GL_EXT_blend_equation_separate_OGLEXT

   GLvoid            WRAPPER02V(BlendEquationSeparateEXT, GLenum, GLenum)

#endif /* GL_EXT_blend_equation_separate_OGLEXT */

// ---[ GL_EXT_blend_func_separate ]----------------------------------------------------------------------------

#ifdef GL_EXT_blend_func_separate_OGLEXT

   GLvoid            WRAPPER04V(BlendFuncSeparateEXT, GLenum, GLenum, GLenum, GLenum)

#endif /* GL_EXT_blend_func_separate_OGLEXT */

// ---[ GL_EXT_blend_minmax ]-----------------------------------------------------------------------------------

#ifdef GL_EXT_blend_minmax_OGLEXT

   GLvoid            WRAPPER01V(BlendEquationEXT, GLenum)

#endif /* GL_EXT_blend_minmax_OGLEXT */

// ---[ GL_EXT_color_subtable ]---------------------------------------------------------------------------------

#ifdef GL_EXT_color_subtable_OGLEXT

   GLvoid            WRAPPER06V(ColorSubTableEXT, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER05V(CopyColorSubTableEXT, GLenum, GLsizei, GLint, GLint, GLsizei)

#endif /* GL_EXT_color_subtable_OGLEXT */

// ---[ GL_EXT_compiled_vertex_array ]--------------------------------------------------------------------------

#ifdef GL_EXT_compiled_vertex_array_OGLEXT

   GLvoid            WRAPPER02V(LockArraysEXT, GLint, GLsizei)
   GLvoid            WRAPPER00V(UnlockArraysEXT)

#endif /* GL_EXT_compiled_vertex_array_OGLEXT */

// ---[ GL_EXT_convolution ]------------------------------------------------------------------------------------

#ifdef GL_EXT_convolution_OGLEXT

   GLvoid            WRAPPER06V(ConvolutionFilter1DEXT, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER07V(ConvolutionFilter2DEXT, GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER03V(ConvolutionParameterfEXT, GLenum, GLenum, GLfloat)
   GLvoid            WRAPPER03V(ConvolutionParameterfvEXT, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(ConvolutionParameteriEXT, GLenum, GLenum, GLint)
   GLvoid            WRAPPER03V(ConvolutionParameterivEXT, GLenum, GLenum, GLint const *)
   GLvoid            WRAPPER05V(CopyConvolutionFilter1DEXT, GLenum, GLenum, GLint, GLint, GLsizei)
   GLvoid            WRAPPER06V(CopyConvolutionFilter2DEXT, GLenum, GLenum, GLint, GLint, GLsizei, GLsizei)
   GLvoid            WRAPPER04V(GetConvolutionFilterEXT, GLenum, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetConvolutionParameterfvEXT, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetConvolutionParameterivEXT, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER06V(GetSeparableFilterEXT, GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *)
   GLvoid            WRAPPER08V(SeparableFilter2DEXT, GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *, GLvoid const *)

#endif /* GL_EXT_convolution_OGLEXT */

// ---[ GL_EXT_coordinate_frame ]-------------------------------------------------------------------------------

#ifdef GL_EXT_coordinate_frame_OGLEXT

   GLvoid            WRAPPER03V(Binormal3bEXT, GLbyte, GLbyte, GLbyte)
   GLvoid            WRAPPER01V(Binormal3bvEXT, GLbyte const *)
   GLvoid            WRAPPER03V(Binormal3dEXT, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(Binormal3dvEXT, GLdouble const *)
   GLvoid            WRAPPER03V(Binormal3fEXT, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(Binormal3fvEXT, GLfloat const *)
   GLvoid            WRAPPER03V(Binormal3iEXT, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(Binormal3ivEXT, GLint const *)
   GLvoid            WRAPPER03V(Binormal3sEXT, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(Binormal3svEXT, GLshort const *)
   GLvoid            WRAPPER03V(BinormalPointerEXT, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER03V(Tangent3bEXT, GLbyte, GLbyte, GLbyte)
   GLvoid            WRAPPER01V(Tangent3bvEXT, GLbyte const *)
   GLvoid            WRAPPER03V(Tangent3dEXT, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(Tangent3dvEXT, GLdouble const *)
   GLvoid            WRAPPER03V(Tangent3fEXT, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(Tangent3fvEXT, GLfloat const *)
   GLvoid            WRAPPER03V(Tangent3iEXT, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(Tangent3ivEXT, GLint const *)
   GLvoid            WRAPPER03V(Tangent3sEXT, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(Tangent3svEXT, GLshort const *)
   GLvoid            WRAPPER03V(TangentPointerEXT, GLenum, GLsizei, GLvoid const *)

#endif /* GL_EXT_coordinate_frame_OGLEXT */

// ---[ GL_EXT_copy_texture ]-----------------------------------------------------------------------------------

#ifdef GL_EXT_copy_texture_OGLEXT

   GLvoid            WRAPPER07V(CopyTexImage1DEXT, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint)
   GLvoid            WRAPPER08V(CopyTexImage2DEXT, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint)
   GLvoid            WRAPPER06V(CopyTexSubImage1DEXT, GLenum, GLint, GLint, GLint, GLint, GLsizei)
   GLvoid            WRAPPER08V(CopyTexSubImage2DEXT, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)
   GLvoid            WRAPPER09V(CopyTexSubImage3DEXT, GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)

#endif /* GL_EXT_copy_texture_OGLEXT */

// ---[ GL_EXT_cull_vertex ]------------------------------------------------------------------------------------

#ifdef GL_EXT_cull_vertex_OGLEXT

   GLvoid            WRAPPER02V(CullParameterdvEXT, GLenum, GLdouble *)
   GLvoid            WRAPPER02V(CullParameterfvEXT, GLenum, GLfloat *)

#endif /* GL_EXT_cull_vertex_OGLEXT */

// ---[ GL_EXT_depth_bounds_test ]------------------------------------------------------------------------------

#ifdef GL_EXT_depth_bounds_test_OGLEXT

   GLvoid            WRAPPER02V(DepthBoundsEXT, GLclampd, GLclampd)

#endif /* GL_EXT_depth_bounds_test_OGLEXT */

// ---[ GL_EXT_draw_range_elements ]----------------------------------------------------------------------------

#ifdef GL_EXT_draw_range_elements_OGLEXT

   GLvoid            WRAPPER06V(DrawRangeElementsEXT, GLenum, GLuint, GLuint, GLsizei, GLenum, GLvoid const *)

#endif /* GL_EXT_draw_range_elements_OGLEXT */

// ---[ GL_EXT_fog_coord ]--------------------------------------------------------------------------------------

#ifdef GL_EXT_fog_coord_OGLEXT

   GLvoid            WRAPPER01V(FogCoorddEXT, GLdouble)
   GLvoid            WRAPPER01V(FogCoorddvEXT, GLdouble const *)
   GLvoid            WRAPPER01V(FogCoordfEXT, GLfloat)
   GLvoid            WRAPPER01V(FogCoordfvEXT, GLfloat const *)
   GLvoid            WRAPPER03V(FogCoordPointerEXT, GLenum, GLsizei, GLvoid const *)

#endif /* GL_EXT_fog_coord_OGLEXT */

// ---[ GL_EXT_framebuffer_object ]-----------------------------------------------------------------------------

#ifdef GL_EXT_framebuffer_object_OGLEXT

   GLvoid            WRAPPER02V(BindFramebufferEXT, GLenum, GLuint)
   GLvoid            WRAPPER02V(BindRenderbufferEXT, GLenum, GLuint)
   GLenum            WRAPPER01R(CheckFramebufferStatusEXT, GLenum)
   GLvoid            WRAPPER02V(DeleteFramebuffersEXT, GLsizei, GLuint const *)
   GLvoid            WRAPPER02V(DeleteRenderbuffersEXT, GLsizei, GLuint const *)
   GLvoid            WRAPPER04V(FramebufferRenderbufferEXT, GLenum, GLenum, GLenum, GLuint)
   GLvoid            WRAPPER05V(FramebufferTexture1DEXT, GLenum, GLenum, GLenum, GLuint, GLint)
   GLvoid            WRAPPER05V(FramebufferTexture2DEXT, GLenum, GLenum, GLenum, GLuint, GLint)
   GLvoid            WRAPPER06V(FramebufferTexture3DEXT, GLenum, GLenum, GLenum, GLuint, GLint, GLint)
   GLvoid            WRAPPER01V(GenerateMipmapEXT, GLenum)
   GLvoid            WRAPPER02V(GenFramebuffersEXT, GLsizei, GLuint *)
   GLvoid            WRAPPER02V(GenRenderbuffersEXT, GLsizei, GLuint *)
   GLvoid            WRAPPER04V(GetFramebufferAttachmentParameterivEXT, GLenum, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetRenderbufferParameterivEXT, GLenum, GLenum, GLint *)
   GLboolean         WRAPPER01R(IsFramebufferEXT, GLuint)
   GLboolean         WRAPPER01R(IsRenderbufferEXT, GLuint)
   GLvoid            WRAPPER04V(RenderbufferStorageEXT, GLenum, GLenum, GLsizei, GLsizei)

#endif /* GL_EXT_framebuffer_object_OGLEXT */

// ---[ GL_EXT_histogram ]--------------------------------------------------------------------------------------

#ifdef GL_EXT_histogram_OGLEXT

   GLvoid            WRAPPER05V(GetHistogramEXT, GLenum, GLboolean, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetHistogramParameterfvEXT, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetHistogramParameterivEXT, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER05V(GetMinmaxEXT, GLenum, GLboolean, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetMinmaxParameterfvEXT, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetMinmaxParameterivEXT, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER04V(HistogramEXT, GLenum, GLsizei, GLenum, GLboolean)
   GLvoid            WRAPPER03V(MinmaxEXT, GLenum, GLenum, GLboolean)
   GLvoid            WRAPPER01V(ResetHistogramEXT, GLenum)
   GLvoid            WRAPPER01V(ResetMinmaxEXT, GLenum)

#endif /* GL_EXT_histogram_OGLEXT */

// ---[ GL_EXT_index_func ]-------------------------------------------------------------------------------------

#ifdef GL_EXT_index_func_OGLEXT

   GLvoid            WRAPPER02V(IndexFuncEXT, GLenum, GLclampf)

#endif /* GL_EXT_index_func_OGLEXT */

// ---[ GL_EXT_index_material ]---------------------------------------------------------------------------------

#ifdef GL_EXT_index_material_OGLEXT

   GLvoid            WRAPPER02V(IndexMaterialEXT, GLenum, GLenum)

#endif /* GL_EXT_index_material_OGLEXT */

// ---[ GL_EXT_light_texture ]----------------------------------------------------------------------------------

#ifdef GL_EXT_light_texture_OGLEXT

   GLvoid            WRAPPER01V(ApplyTextureEXT, GLenum)
   GLvoid            WRAPPER01V(TextureLightEXT, GLenum)
   GLvoid            WRAPPER02V(TextureMaterialEXT, GLenum, GLenum)

#endif /* GL_EXT_light_texture_OGLEXT */

// ---[ GL_EXT_multi_draw_arrays ]------------------------------------------------------------------------------

#ifdef GL_EXT_multi_draw_arrays_OGLEXT

   GLvoid            WRAPPER04V(MultiDrawArraysEXT, GLenum, GLint *, GLsizei *, GLsizei)
   GLvoid            WRAPPER05V(MultiDrawElementsEXT, GLenum, GLsizei const *, GLenum, GLvoid const * *, GLsizei)

#endif /* GL_EXT_multi_draw_arrays_OGLEXT */

// ---[ GL_EXT_multisample ]------------------------------------------------------------------------------------

#ifdef GL_EXT_multisample_OGLEXT

   GLvoid            WRAPPER02V(SampleMaskEXT, GLclampf, GLboolean)
   GLvoid            WRAPPER01V(SamplePatternEXT, GLenum)

#endif /* GL_EXT_multisample_OGLEXT */

// ---[ GL_EXT_paletted_texture ]-------------------------------------------------------------------------------

#ifdef GL_EXT_paletted_texture_OGLEXT

   GLvoid            WRAPPER06V(ColorSubTableEXT, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER06V(ColorTableEXT, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER04V(GetColorTableEXT, GLenum, GLenum, GLenum, GLvoid *)
   GLvoid            WRAPPER03V(GetColorTableParameterfvEXT, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetColorTableParameterivEXT, GLenum, GLenum, GLint *)

#endif /* GL_EXT_paletted_texture_OGLEXT */

// ---[ GL_EXT_pixel_transform ]--------------------------------------------------------------------------------

#ifdef GL_EXT_pixel_transform_OGLEXT

   GLvoid            WRAPPER03V(PixelTransformParameterfEXT, GLenum, GLenum, GLfloat)
   GLvoid            WRAPPER03V(PixelTransformParameterfvEXT, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(PixelTransformParameteriEXT, GLenum, GLenum, GLint)
   GLvoid            WRAPPER03V(PixelTransformParameterivEXT, GLenum, GLenum, GLint const *)

#endif /* GL_EXT_pixel_transform_OGLEXT */

// ---[ GL_EXT_point_parameters ]-------------------------------------------------------------------------------

#ifdef GL_EXT_point_parameters_OGLEXT

   GLvoid            WRAPPER02V(PointParameterfEXT, GLenum, GLfloat)
   GLvoid            WRAPPER02V(PointParameterfvEXT, GLenum, GLfloat const *)

#endif /* GL_EXT_point_parameters_OGLEXT */

// ---[ GL_EXT_polygon_offset ]---------------------------------------------------------------------------------

#ifdef GL_EXT_polygon_offset_OGLEXT

   GLvoid            WRAPPER02V(PolygonOffsetEXT, GLfloat, GLfloat)

#endif /* GL_EXT_polygon_offset_OGLEXT */

// ---[ GL_EXT_secondary_color ]--------------------------------------------------------------------------------

#ifdef GL_EXT_secondary_color_OGLEXT

   GLvoid            WRAPPER03V(SecondaryColor3bEXT, GLbyte, GLbyte, GLbyte)
   GLvoid            WRAPPER01V(SecondaryColor3bvEXT, GLbyte const *)
   GLvoid            WRAPPER03V(SecondaryColor3dEXT, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(SecondaryColor3dvEXT, GLdouble const *)
   GLvoid            WRAPPER03V(SecondaryColor3fEXT, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(SecondaryColor3fvEXT, GLfloat const *)
   GLvoid            WRAPPER03V(SecondaryColor3iEXT, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(SecondaryColor3ivEXT, GLint const *)
   GLvoid            WRAPPER03V(SecondaryColor3sEXT, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(SecondaryColor3svEXT, GLshort const *)
   GLvoid            WRAPPER03V(SecondaryColor3ubEXT, GLubyte, GLubyte, GLubyte)
   GLvoid            WRAPPER01V(SecondaryColor3ubvEXT, GLubyte const *)
   GLvoid            WRAPPER03V(SecondaryColor3uiEXT, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER01V(SecondaryColor3uivEXT, GLuint const *)
   GLvoid            WRAPPER03V(SecondaryColor3usEXT, GLushort, GLushort, GLushort)
   GLvoid            WRAPPER01V(SecondaryColor3usvEXT, GLushort const *)
   GLvoid            WRAPPER04V(SecondaryColorPointerEXT, GLint, GLenum, GLsizei, GLvoid const *)

#endif /* GL_EXT_secondary_color_OGLEXT */

// ---[ GL_EXT_stencil_two_side ]-------------------------------------------------------------------------------

#ifdef GL_EXT_stencil_two_side_OGLEXT

   GLvoid            WRAPPER01V(ActiveStencilFaceEXT, GLenum)

#endif /* GL_EXT_stencil_two_side_OGLEXT */

// ---[ GL_EXT_subtexture ]-------------------------------------------------------------------------------------

#ifdef GL_EXT_subtexture_OGLEXT

   GLvoid            WRAPPER07V(TexSubImage1DEXT, GLenum, GLint, GLint, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER09V(TexSubImage2DEXT, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)

#endif /* GL_EXT_subtexture_OGLEXT */

// ---[ GL_EXT_texture3D ]--------------------------------------------------------------------------------------

#ifdef GL_EXT_texture3D_OGLEXT

   GLvoid            WRAPPER10V(TexImage3DEXT, GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER11V(TexSubImage3DEXT, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)

#endif /* GL_EXT_texture3D_OGLEXT */

// ---[ GL_EXT_texture_object ]---------------------------------------------------------------------------------

#ifdef GL_EXT_texture_object_OGLEXT

   GLboolean         WRAPPER03R(AreTexturesResidentEXT, GLsizei, GLuint const *, GLboolean *)
   GLvoid            WRAPPER02V(BindTextureEXT, GLenum, GLuint)
   GLvoid            WRAPPER02V(DeleteTexturesEXT, GLsizei, GLuint const *)
   GLvoid            WRAPPER02V(GenTexturesEXT, GLsizei, GLuint *)
   GLboolean         WRAPPER01R(IsTextureEXT, GLuint)
   GLvoid            WRAPPER03V(PrioritizeTexturesEXT, GLsizei, GLuint const *, GLclampf const *)

#endif /* GL_EXT_texture_object_OGLEXT */

// ---[ GL_EXT_texture_perturb_normal ]-------------------------------------------------------------------------

#ifdef GL_EXT_texture_perturb_normal_OGLEXT

   GLvoid            WRAPPER01V(TextureNormalEXT, GLenum)

#endif /* GL_EXT_texture_perturb_normal_OGLEXT */

// ---[ GL_EXT_vertex_array ]-----------------------------------------------------------------------------------

#ifdef GL_EXT_vertex_array_OGLEXT

   GLvoid            WRAPPER01V(ArrayElementEXT, GLint)
   GLvoid            WRAPPER05V(ColorPointerEXT, GLint, GLenum, GLsizei, GLsizei, GLvoid const *)
   GLvoid            WRAPPER03V(DrawArraysEXT, GLenum, GLint, GLsizei)
   GLvoid            WRAPPER03V(EdgeFlagPointerEXT, GLsizei, GLsizei, GLboolean const *)
   GLvoid            WRAPPER02V(GetPointervEXT, GLenum, GLvoid * *)
   GLvoid            WRAPPER04V(IndexPointerEXT, GLenum, GLsizei, GLsizei, GLvoid const *)
   GLvoid            WRAPPER04V(NormalPointerEXT, GLenum, GLsizei, GLsizei, GLvoid const *)
   GLvoid            WRAPPER05V(TexCoordPointerEXT, GLint, GLenum, GLsizei, GLsizei, GLvoid const *)
   GLvoid            WRAPPER05V(VertexPointerEXT, GLint, GLenum, GLsizei, GLsizei, GLvoid const *)

#endif /* GL_EXT_vertex_array_OGLEXT */

// ---[ GL_EXT_vertex_shader ]----------------------------------------------------------------------------------

#ifdef GL_EXT_vertex_shader_OGLEXT

   GLvoid            WRAPPER00V(BeginVertexShaderEXT)
   GLuint            WRAPPER02R(BindLightParameterEXT, GLenum, GLenum)
   GLuint            WRAPPER02R(BindMaterialParameterEXT, GLenum, GLenum)
   GLuint            WRAPPER01R(BindParameterEXT, GLenum)
   GLuint            WRAPPER03R(BindTexGenParameterEXT, GLenum, GLenum, GLenum)
   GLuint            WRAPPER02R(BindTextureUnitParameterEXT, GLenum, GLenum)
   GLvoid            WRAPPER01V(BindVertexShaderEXT, GLuint)
   GLvoid            WRAPPER01V(DeleteVertexShaderEXT, GLuint)
   GLvoid            WRAPPER01V(DisableVariantClientStateEXT, GLuint)
   GLvoid            WRAPPER01V(EnableVariantClientStateEXT, GLuint)
   GLvoid            WRAPPER00V(EndVertexShaderEXT)
   GLvoid            WRAPPER03V(ExtractComponentEXT, GLuint, GLuint, GLuint)
   GLuint            WRAPPER04R(GenSymbolsEXT, GLenum, GLenum, GLenum, GLuint)
   GLuint            WRAPPER01R(GenVertexShadersEXT, GLuint)
   GLvoid            WRAPPER03V(GetInvariantBooleanvEXT, GLuint, GLenum, GLboolean *)
   GLvoid            WRAPPER03V(GetInvariantFloatvEXT, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetInvariantIntegervEXT, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetLocalConstantBooleanvEXT, GLuint, GLenum, GLboolean *)
   GLvoid            WRAPPER03V(GetLocalConstantFloatvEXT, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetLocalConstantIntegervEXT, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetVariantBooleanvEXT, GLuint, GLenum, GLboolean *)
   GLvoid            WRAPPER03V(GetVariantFloatvEXT, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetVariantIntegervEXT, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetVariantPointervEXT, GLuint, GLenum, GLvoid * *)
   GLvoid            WRAPPER03V(InsertComponentEXT, GLuint, GLuint, GLuint)
   GLboolean         WRAPPER02R(IsVariantEnabledEXT, GLuint, GLenum)
   GLvoid            WRAPPER03V(SetInvariantEXT, GLuint, GLenum, GLvoid const *)
   GLvoid            WRAPPER03V(SetLocalConstantEXT, GLuint, GLenum, GLvoid const *)
   GLvoid            WRAPPER03V(ShaderOp1EXT, GLenum, GLuint, GLuint)
   GLvoid            WRAPPER04V(ShaderOp2EXT, GLenum, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER05V(ShaderOp3EXT, GLenum, GLuint, GLuint, GLuint, GLuint)
   GLvoid            WRAPPER06V(SwizzleEXT, GLuint, GLuint, GLenum, GLenum, GLenum, GLenum)
   GLvoid            WRAPPER02V(VariantbvEXT, GLuint, GLbyte const *)
   GLvoid            WRAPPER02V(VariantdvEXT, GLuint, GLdouble const *)
   GLvoid            WRAPPER02V(VariantfvEXT, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(VariantivEXT, GLuint, GLint const *)
   GLvoid            WRAPPER04V(VariantPointerEXT, GLuint, GLenum, GLuint, GLvoid const *)
   GLvoid            WRAPPER02V(VariantsvEXT, GLuint, GLshort const *)
   GLvoid            WRAPPER02V(VariantubvEXT, GLuint, GLubyte const *)
   GLvoid            WRAPPER02V(VariantuivEXT, GLuint, GLuint const *)
   GLvoid            WRAPPER02V(VariantusvEXT, GLuint, GLushort const *)
   GLvoid            WRAPPER06V(WriteMaskEXT, GLuint, GLuint, GLenum, GLenum, GLenum, GLenum)

#endif /* GL_EXT_vertex_shader_OGLEXT */

// ---[ GL_EXT_vertex_weighting ]-------------------------------------------------------------------------------

#ifdef GL_EXT_vertex_weighting_OGLEXT

   GLvoid            WRAPPER01V(VertexWeightfEXT, GLfloat)
   GLvoid            WRAPPER01V(VertexWeightfvEXT, GLfloat const *)
   GLvoid            WRAPPER04V(VertexWeightPointerEXT, GLsizei, GLenum, GLsizei, GLvoid const *)

#endif /* GL_EXT_vertex_weighting_OGLEXT */

// ---[ GL_GREMEDY_string_marker ]------------------------------------------------------------------------------

#ifdef GL_GREMEDY_string_marker_OGLEXT

   GLvoid            WRAPPER02V(StringMarkerGREMEDY, GLsizei, GLvoid const *)

#endif /* GL_GREMEDY_string_marker_OGLEXT */

// ---[ GL_HP_image_transform ]---------------------------------------------------------------------------------

#ifdef GL_HP_image_transform_OGLEXT

   GLvoid            WRAPPER03V(GetImageTransformParameterfvHP, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetImageTransformParameterivHP, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(ImageTransformParameterfHP, GLenum, GLenum, GLfloat)
   GLvoid            WRAPPER03V(ImageTransformParameterfvHP, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(ImageTransformParameteriHP, GLenum, GLenum, GLint)
   GLvoid            WRAPPER03V(ImageTransformParameterivHP, GLenum, GLenum, GLint const *)

#endif /* GL_HP_image_transform_OGLEXT */

// ---[ GL_IBM_multimode_draw_arrays ]--------------------------------------------------------------------------

#ifdef GL_IBM_multimode_draw_arrays_OGLEXT

   GLvoid            WRAPPER05V(MultiModeDrawArraysIBM, GLenum const *, GLint const *, GLsizei const *, GLsizei, GLint)
   GLvoid            WRAPPER06V(MultiModeDrawElementsIBM, GLenum const *, GLsizei const *, GLenum, GLvoid const * const *, GLsizei, GLint)

#endif /* GL_IBM_multimode_draw_arrays_OGLEXT */

// ---[ GL_IBM_vertex_array_lists ]-----------------------------------------------------------------------------

#ifdef GL_IBM_vertex_array_lists_OGLEXT

   GLvoid            WRAPPER05V(ColorPointerListIBM, GLint, GLenum, GLint, GLvoid const * *, GLint)
   GLvoid            WRAPPER03V(EdgeFlagPointerListIBM, GLint, GLboolean const * *, GLint)
   GLvoid            WRAPPER04V(FogCoordPointerListIBM, GLenum, GLint, GLvoid const * *, GLint)
   GLvoid            WRAPPER04V(IndexPointerListIBM, GLenum, GLint, GLvoid const * *, GLint)
   GLvoid            WRAPPER04V(NormalPointerListIBM, GLenum, GLint, GLvoid const * *, GLint)
   GLvoid            WRAPPER05V(SecondaryColorPointerListIBM, GLint, GLenum, GLint, GLvoid const * *, GLint)
   GLvoid            WRAPPER05V(TexCoordPointerListIBM, GLint, GLenum, GLint, GLvoid const * *, GLint)
   GLvoid            WRAPPER05V(VertexPointerListIBM, GLint, GLenum, GLint, GLvoid const * *, GLint)

#endif /* GL_IBM_vertex_array_lists_OGLEXT */

// ---[ GL_INGR_blend_func_separate ]---------------------------------------------------------------------------

#ifdef GL_INGR_blend_func_separate_OGLEXT

   GLvoid            WRAPPER04V(BlendFuncSeparateINGR, GLenum, GLenum, GLenum, GLenum)

#endif /* GL_INGR_blend_func_separate_OGLEXT */

// ---[ GL_INTEL_parallel_arrays ]------------------------------------------------------------------------------

#ifdef GL_INTEL_parallel_arrays_OGLEXT

   GLvoid            WRAPPER03V(ColorPointervINTEL, GLint, GLenum, GLvoid const * *)
   GLvoid            WRAPPER02V(NormalPointervINTEL, GLenum, GLvoid const * *)
   GLvoid            WRAPPER03V(TexCoordPointervINTEL, GLint, GLenum, GLvoid const * *)
   GLvoid            WRAPPER03V(VertexPointervINTEL, GLint, GLenum, GLvoid const * *)

#endif /* GL_INTEL_parallel_arrays_OGLEXT */

// ---[ GL_MESA_resize_buffers ]--------------------------------------------------------------------------------

#ifdef GL_MESA_resize_buffers_OGLEXT

   GLvoid            WRAPPER00V(ResizeBuffersMESA)

#endif /* GL_MESA_resize_buffers_OGLEXT */

// ---[ GL_MESA_window_pos ]------------------------------------------------------------------------------------

#ifdef GL_MESA_window_pos_OGLEXT

   GLvoid            WRAPPER02V(WindowPos2dMESA, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(WindowPos2dvMESA, GLdouble const *)
   GLvoid            WRAPPER02V(WindowPos2fMESA, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(WindowPos2fvMESA, GLfloat const *)
   GLvoid            WRAPPER02V(WindowPos2iMESA, GLint, GLint)
   GLvoid            WRAPPER01V(WindowPos2ivMESA, GLint const *)
   GLvoid            WRAPPER02V(WindowPos2sMESA, GLshort, GLshort)
   GLvoid            WRAPPER01V(WindowPos2svMESA, GLshort const *)
   GLvoid            WRAPPER03V(WindowPos3dMESA, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(WindowPos3dvMESA, GLdouble const *)
   GLvoid            WRAPPER03V(WindowPos3fMESA, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(WindowPos3fvMESA, GLfloat const *)
   GLvoid            WRAPPER03V(WindowPos3iMESA, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(WindowPos3ivMESA, GLint const *)
   GLvoid            WRAPPER03V(WindowPos3sMESA, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(WindowPos3svMESA, GLshort const *)
   GLvoid            WRAPPER04V(WindowPos4dMESA, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER01V(WindowPos4dvMESA, GLdouble const *)
   GLvoid            WRAPPER04V(WindowPos4fMESA, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER01V(WindowPos4fvMESA, GLfloat const *)
   GLvoid            WRAPPER04V(WindowPos4iMESA, GLint, GLint, GLint, GLint)
   GLvoid            WRAPPER01V(WindowPos4ivMESA, GLint const *)
   GLvoid            WRAPPER04V(WindowPos4sMESA, GLshort, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER01V(WindowPos4svMESA, GLshort const *)

#endif /* GL_MESA_window_pos_OGLEXT */

// ---[ GL_NV_element_array ]-----------------------------------------------------------------------------------

#ifdef GL_NV_element_array_OGLEXT

   GLvoid            WRAPPER03V(DrawElementArrayNV, GLenum, GLint, GLsizei)
   GLvoid            WRAPPER05V(DrawRangeElementArrayNV, GLenum, GLuint, GLuint, GLint, GLsizei)
   GLvoid            WRAPPER02V(ElementPointerNV, GLenum, GLvoid const *)
   GLvoid            WRAPPER04V(MultiDrawElementArrayNV, GLenum, GLint const *, GLsizei const *, GLsizei)
   GLvoid            WRAPPER06V(MultiDrawRangeElementArrayNV, GLenum, GLuint, GLuint, GLint const *, GLsizei const *, GLsizei)

#endif /* GL_NV_element_array_OGLEXT */

// ---[ GL_NV_evaluators ]--------------------------------------------------------------------------------------

#ifdef GL_NV_evaluators_OGLEXT

   GLvoid            WRAPPER02V(EvalMapsNV, GLenum, GLenum)
   GLvoid            WRAPPER04V(GetMapAttribParameterfvNV, GLenum, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER04V(GetMapAttribParameterivNV, GLenum, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER07V(GetMapControlPointsNV, GLenum, GLuint, GLenum, GLsizei, GLsizei, GLboolean, GLvoid *)
   GLvoid            WRAPPER03V(GetMapParameterfvNV, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetMapParameterivNV, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER09V(MapControlPointsNV, GLenum, GLuint, GLenum, GLsizei, GLsizei, GLint, GLint, GLboolean, GLvoid const *)
   GLvoid            WRAPPER03V(MapParameterfvNV, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(MapParameterivNV, GLenum, GLenum, GLint const *)

#endif /* GL_NV_evaluators_OGLEXT */

// ---[ GL_NV_fence ]-------------------------------------------------------------------------------------------

#ifdef GL_NV_fence_OGLEXT

   GLvoid            WRAPPER02V(DeleteFencesNV, GLsizei, GLuint const *)
   GLvoid            WRAPPER01V(FinishFenceNV, GLuint)
   GLvoid            WRAPPER02V(GenFencesNV, GLsizei, GLuint *)
   GLvoid            WRAPPER03V(GetFenceivNV, GLuint, GLenum, GLint *)
   GLboolean         WRAPPER01R(IsFenceNV, GLuint)
   GLvoid            WRAPPER02V(SetFenceNV, GLuint, GLenum)
   GLboolean         WRAPPER01R(TestFenceNV, GLuint)

#endif /* GL_NV_fence_OGLEXT */

// ---[ GL_NV_fragment_program ]--------------------------------------------------------------------------------

#ifdef GL_NV_fragment_program_OGLEXT

   GLvoid            WRAPPER04V(GetProgramNamedParameterdvNV, GLuint, GLsizei, GLubyte const *, GLdouble *)
   GLvoid            WRAPPER04V(GetProgramNamedParameterfvNV, GLuint, GLsizei, GLubyte const *, GLfloat *)
   GLvoid            WRAPPER07V(ProgramNamedParameter4dNV, GLuint, GLsizei, GLubyte const *, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER04V(ProgramNamedParameter4dvNV, GLuint, GLsizei, GLubyte const *, GLdouble const *)
   GLvoid            WRAPPER07V(ProgramNamedParameter4fNV, GLuint, GLsizei, GLubyte const *, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER04V(ProgramNamedParameter4fvNV, GLuint, GLsizei, GLubyte const *, GLfloat const *)

#endif /* GL_NV_fragment_program_OGLEXT */

// ---[ GL_NV_half_float ]--------------------------------------------------------------------------------------

#ifdef GL_NV_half_float_OGLEXT

   GLvoid            WRAPPER03V(Color3hNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(Color3hvNV, GLhalfNV const *)
   GLvoid            WRAPPER04V(Color4hNV, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(Color4hvNV, GLhalfNV const *)
   GLvoid            WRAPPER01V(FogCoordhNV, GLhalfNV)
   GLvoid            WRAPPER01V(FogCoordhvNV, GLhalfNV const *)
   GLvoid            WRAPPER02V(MultiTexCoord1hNV, GLenum, GLhalfNV)
   GLvoid            WRAPPER02V(MultiTexCoord1hvNV, GLenum, GLhalfNV const *)
   GLvoid            WRAPPER03V(MultiTexCoord2hNV, GLenum, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER02V(MultiTexCoord2hvNV, GLenum, GLhalfNV const *)
   GLvoid            WRAPPER04V(MultiTexCoord3hNV, GLenum, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER02V(MultiTexCoord3hvNV, GLenum, GLhalfNV const *)
   GLvoid            WRAPPER05V(MultiTexCoord4hNV, GLenum, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER02V(MultiTexCoord4hvNV, GLenum, GLhalfNV const *)
   GLvoid            WRAPPER03V(Normal3hNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(Normal3hvNV, GLhalfNV const *)
   GLvoid            WRAPPER03V(SecondaryColor3hNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(SecondaryColor3hvNV, GLhalfNV const *)
   GLvoid            WRAPPER01V(TexCoord1hNV, GLhalfNV)
   GLvoid            WRAPPER01V(TexCoord1hvNV, GLhalfNV const *)
   GLvoid            WRAPPER02V(TexCoord2hNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(TexCoord2hvNV, GLhalfNV const *)
   GLvoid            WRAPPER03V(TexCoord3hNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(TexCoord3hvNV, GLhalfNV const *)
   GLvoid            WRAPPER04V(TexCoord4hNV, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(TexCoord4hvNV, GLhalfNV const *)
   GLvoid            WRAPPER02V(Vertex2hNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(Vertex2hvNV, GLhalfNV const *)
   GLvoid            WRAPPER03V(Vertex3hNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(Vertex3hvNV, GLhalfNV const *)
   GLvoid            WRAPPER04V(Vertex4hNV, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER01V(Vertex4hvNV, GLhalfNV const *)
   GLvoid            WRAPPER02V(VertexAttrib1hNV, GLuint, GLhalfNV)
   GLvoid            WRAPPER02V(VertexAttrib1hvNV, GLuint, GLhalfNV const *)
   GLvoid            WRAPPER03V(VertexAttrib2hNV, GLuint, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER02V(VertexAttrib2hvNV, GLuint, GLhalfNV const *)
   GLvoid            WRAPPER04V(VertexAttrib3hNV, GLuint, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER02V(VertexAttrib3hvNV, GLuint, GLhalfNV const *)
   GLvoid            WRAPPER05V(VertexAttrib4hNV, GLuint, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV)
   GLvoid            WRAPPER02V(VertexAttrib4hvNV, GLuint, GLhalfNV const *)
   GLvoid            WRAPPER03V(VertexAttribs1hvNV, GLuint, GLsizei, GLhalfNV const *)
   GLvoid            WRAPPER03V(VertexAttribs2hvNV, GLuint, GLsizei, GLhalfNV const *)
   GLvoid            WRAPPER03V(VertexAttribs3hvNV, GLuint, GLsizei, GLhalfNV const *)
   GLvoid            WRAPPER03V(VertexAttribs4hvNV, GLuint, GLsizei, GLhalfNV const *)
   GLvoid            WRAPPER01V(VertexWeighthNV, GLhalfNV)
   GLvoid            WRAPPER01V(VertexWeighthvNV, GLhalfNV const *)

#endif /* GL_NV_half_float_OGLEXT */

// ---[ GL_NV_occlusion_query ]---------------------------------------------------------------------------------

#ifdef GL_NV_occlusion_query_OGLEXT

   GLvoid            WRAPPER01V(BeginOcclusionQueryNV, GLuint)
   GLvoid            WRAPPER02V(DeleteOcclusionQueriesNV, GLsizei, GLuint const *)
   GLvoid            WRAPPER00V(EndOcclusionQueryNV)
   GLvoid            WRAPPER02V(GenOcclusionQueriesNV, GLsizei, GLuint *)
   GLvoid            WRAPPER03V(GetOcclusionQueryivNV, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetOcclusionQueryuivNV, GLuint, GLenum, GLuint *)
   GLboolean         WRAPPER01R(IsOcclusionQueryNV, GLuint)

#endif /* GL_NV_occlusion_query_OGLEXT */

// ---[ GL_NV_pixel_data_range ]--------------------------------------------------------------------------------

#ifdef GL_NV_pixel_data_range_OGLEXT

   GLvoid            WRAPPER01V(FlushPixelDataRangeNV, GLenum)
   GLvoid            WRAPPER03V(PixelDataRangeNV, GLenum, GLsizei, GLvoid *)

#endif /* GL_NV_pixel_data_range_OGLEXT */

// ---[ GL_NV_point_sprite ]------------------------------------------------------------------------------------

#ifdef GL_NV_point_sprite_OGLEXT

   GLvoid            WRAPPER02V(PointParameteriNV, GLenum, GLint)
   GLvoid            WRAPPER02V(PointParameterivNV, GLenum, GLint const *)

#endif /* GL_NV_point_sprite_OGLEXT */

// ---[ GL_NV_primitive_restart ]-------------------------------------------------------------------------------

#ifdef GL_NV_primitive_restart_OGLEXT

   GLvoid            WRAPPER01V(PrimitiveRestartIndexNV, GLuint)
   GLvoid            WRAPPER00V(PrimitiveRestartNV)

#endif /* GL_NV_primitive_restart_OGLEXT */

// ---[ GL_NV_register_combiners ]------------------------------------------------------------------------------

#ifdef GL_NV_register_combiners_OGLEXT

   GLvoid            WRAPPER06V(CombinerInputNV, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum)
   GLvoid            WRAPPER10V(CombinerOutputNV, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLboolean, GLboolean, GLboolean)
   GLvoid            WRAPPER02V(CombinerParameterfNV, GLenum, GLfloat)
   GLvoid            WRAPPER02V(CombinerParameterfvNV, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(CombinerParameteriNV, GLenum, GLint)
   GLvoid            WRAPPER02V(CombinerParameterivNV, GLenum, GLint const *)
   GLvoid            WRAPPER04V(FinalCombinerInputNV, GLenum, GLenum, GLenum, GLenum)
   GLvoid            WRAPPER05V(GetCombinerInputParameterfvNV, GLenum, GLenum, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER05V(GetCombinerInputParameterivNV, GLenum, GLenum, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER04V(GetCombinerOutputParameterfvNV, GLenum, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER04V(GetCombinerOutputParameterivNV, GLenum, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetFinalCombinerInputParameterfvNV, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetFinalCombinerInputParameterivNV, GLenum, GLenum, GLint *)

#endif /* GL_NV_register_combiners_OGLEXT */

// ---[ GL_NV_register_combiners2 ]-----------------------------------------------------------------------------

#ifdef GL_NV_register_combiners2_OGLEXT

   GLvoid            WRAPPER03V(CombinerStageParameterfvNV, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(GetCombinerStageParameterfvNV, GLenum, GLenum, GLfloat *)

#endif /* GL_NV_register_combiners2_OGLEXT */

// ---[ GL_NV_stencil_two_side ]--------------------------------------------------------------------------------

#ifdef GL_NV_stencil_two_side_OGLEXT

   GLvoid            WRAPPER01V(ActiveStencilFaceNV, GLenum)

#endif /* GL_NV_stencil_two_side_OGLEXT */

// ---[ GL_NV_vertex_array_range ]------------------------------------------------------------------------------

#ifdef GL_NV_vertex_array_range_OGLEXT

   GLvoid            WRAPPER00V(FlushVertexArrayRangeNV)
   GLvoid            WRAPPER02V(VertexArrayRangeNV, GLsizei, GLvoid const *)

#endif /* GL_NV_vertex_array_range_OGLEXT */

// ---[ GL_NV_vertex_program ]----------------------------------------------------------------------------------

#ifdef GL_NV_vertex_program_OGLEXT

   GLboolean         WRAPPER03R(AreProgramsResidentNV, GLsizei, GLuint const *, GLboolean *)
   GLvoid            WRAPPER02V(BindProgramNV, GLenum, GLuint)
   GLvoid            WRAPPER02V(DeleteProgramsNV, GLsizei, GLuint const *)
   GLvoid            WRAPPER03V(ExecuteProgramNV, GLenum, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(GenProgramsNV, GLsizei, GLuint *)
   GLvoid            WRAPPER03V(GetProgramivNV, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER04V(GetProgramParameterdvNV, GLenum, GLuint, GLenum, GLdouble *)
   GLvoid            WRAPPER04V(GetProgramParameterfvNV, GLenum, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetProgramStringNV, GLuint, GLenum, GLubyte *)
   GLvoid            WRAPPER04V(GetTrackMatrixivNV, GLenum, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetVertexAttribdvNV, GLuint, GLenum, GLdouble *)
   GLvoid            WRAPPER03V(GetVertexAttribfvNV, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetVertexAttribivNV, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetVertexAttribPointervNV, GLuint, GLenum, GLvoid * *)
   GLboolean         WRAPPER01R(IsProgramNV, GLuint)
   GLvoid            WRAPPER04V(LoadProgramNV, GLenum, GLuint, GLsizei, GLubyte const *)
   GLvoid            WRAPPER06V(ProgramParameter4dNV, GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER03V(ProgramParameter4dvNV, GLenum, GLuint, GLdouble const *)
   GLvoid            WRAPPER06V(ProgramParameter4fNV, GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(ProgramParameter4fvNV, GLenum, GLuint, GLfloat const *)
   GLvoid            WRAPPER04V(ProgramParameters4dvNV, GLenum, GLuint, GLuint, GLdouble const *)
   GLvoid            WRAPPER04V(ProgramParameters4fvNV, GLenum, GLuint, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(RequestResidentProgramsNV, GLsizei, GLuint const *)
   GLvoid            WRAPPER04V(TrackMatrixNV, GLenum, GLuint, GLenum, GLenum)
   GLvoid            WRAPPER02V(VertexAttrib1dNV, GLuint, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib1dvNV, GLuint, GLdouble const *)
   GLvoid            WRAPPER02V(VertexAttrib1fNV, GLuint, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib1fvNV, GLuint, GLfloat const *)
   GLvoid            WRAPPER02V(VertexAttrib1sNV, GLuint, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib1svNV, GLuint, GLshort const *)
   GLvoid            WRAPPER03V(VertexAttrib2dNV, GLuint, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib2dvNV, GLuint, GLdouble const *)
   GLvoid            WRAPPER03V(VertexAttrib2fNV, GLuint, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib2fvNV, GLuint, GLfloat const *)
   GLvoid            WRAPPER03V(VertexAttrib2sNV, GLuint, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib2svNV, GLuint, GLshort const *)
   GLvoid            WRAPPER04V(VertexAttrib3dNV, GLuint, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib3dvNV, GLuint, GLdouble const *)
   GLvoid            WRAPPER04V(VertexAttrib3fNV, GLuint, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib3fvNV, GLuint, GLfloat const *)
   GLvoid            WRAPPER04V(VertexAttrib3sNV, GLuint, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib3svNV, GLuint, GLshort const *)
   GLvoid            WRAPPER05V(VertexAttrib4dNV, GLuint, GLdouble, GLdouble, GLdouble, GLdouble)
   GLvoid            WRAPPER02V(VertexAttrib4dvNV, GLuint, GLdouble const *)
   GLvoid            WRAPPER05V(VertexAttrib4fNV, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(VertexAttrib4fvNV, GLuint, GLfloat const *)
   GLvoid            WRAPPER05V(VertexAttrib4sNV, GLuint, GLshort, GLshort, GLshort, GLshort)
   GLvoid            WRAPPER02V(VertexAttrib4svNV, GLuint, GLshort const *)
   GLvoid            WRAPPER05V(VertexAttrib4ubNV, GLuint, GLubyte, GLubyte, GLubyte, GLubyte)
   GLvoid            WRAPPER02V(VertexAttrib4ubvNV, GLuint, GLubyte const *)
   GLvoid            WRAPPER05V(VertexAttribPointerNV, GLuint, GLint, GLenum, GLsizei, GLvoid const *)
   GLvoid            WRAPPER03V(VertexAttribs1dvNV, GLuint, GLsizei, GLdouble const *)
   GLvoid            WRAPPER03V(VertexAttribs1fvNV, GLuint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER03V(VertexAttribs1svNV, GLuint, GLsizei, GLshort const *)
   GLvoid            WRAPPER03V(VertexAttribs2dvNV, GLuint, GLsizei, GLdouble const *)
   GLvoid            WRAPPER03V(VertexAttribs2fvNV, GLuint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER03V(VertexAttribs2svNV, GLuint, GLsizei, GLshort const *)
   GLvoid            WRAPPER03V(VertexAttribs3dvNV, GLuint, GLsizei, GLdouble const *)
   GLvoid            WRAPPER03V(VertexAttribs3fvNV, GLuint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER03V(VertexAttribs3svNV, GLuint, GLsizei, GLshort const *)
   GLvoid            WRAPPER03V(VertexAttribs4dvNV, GLuint, GLsizei, GLdouble const *)
   GLvoid            WRAPPER03V(VertexAttribs4fvNV, GLuint, GLsizei, GLfloat const *)
   GLvoid            WRAPPER03V(VertexAttribs4svNV, GLuint, GLsizei, GLshort const *)
   GLvoid            WRAPPER03V(VertexAttribs4ubvNV, GLuint, GLsizei, GLubyte const *)

#endif /* GL_NV_vertex_program_OGLEXT */

// ---[ GL_NVX_conditional_render ]-----------------------------------------------------------------------------

#ifdef GL_NVX_conditional_render_OGLEXT

   GLvoid            WRAPPER01V(BeginConditionalRenderNVX, GLuint)
   GLvoid            WRAPPER00V(EndConditionalRenderNVX)

#endif /* GL_NVX_conditional_render_OGLEXT */

// ---[ GL_PGI_misc_hints ]-------------------------------------------------------------------------------------

#ifdef GL_PGI_misc_hints_OGLEXT

   GLvoid            WRAPPER02V(HintPGI, GLenum, GLint)

#endif /* GL_PGI_misc_hints_OGLEXT */

// ---[ GL_SGI_color_table ]------------------------------------------------------------------------------------

#ifdef GL_SGI_color_table_OGLEXT

   GLvoid            WRAPPER03V(ColorTableParameterfvSGI, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(ColorTableParameterivSGI, GLenum, GLenum, GLint const *)
   GLvoid            WRAPPER06V(ColorTableSGI, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER05V(CopyColorTableSGI, GLenum, GLenum, GLint, GLint, GLsizei)
   GLvoid            WRAPPER03V(GetColorTableParameterfvSGI, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetColorTableParameterivSGI, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER04V(GetColorTableSGI, GLenum, GLenum, GLenum, GLvoid *)

#endif /* GL_SGI_color_table_OGLEXT */

// ---[ GL_SGIS_detail_texture ]--------------------------------------------------------------------------------

#ifdef GL_SGIS_detail_texture_OGLEXT

   GLvoid            WRAPPER03V(DetailTexFuncSGIS, GLenum, GLsizei, GLfloat const *)
   GLvoid            WRAPPER02V(GetDetailTexFuncSGIS, GLenum, GLfloat *)

#endif /* GL_SGIS_detail_texture_OGLEXT */

// ---[ GL_SGIS_fog_function ]----------------------------------------------------------------------------------

#ifdef GL_SGIS_fog_function_OGLEXT

   GLvoid            WRAPPER02V(FogFuncSGIS, GLsizei, GLfloat const *)
   GLvoid            WRAPPER01V(GetFogFuncSGIS, GLfloat *)

#endif /* GL_SGIS_fog_function_OGLEXT */

// ---[ GL_SGIS_multisample ]-----------------------------------------------------------------------------------

#ifdef GL_SGIS_multisample_OGLEXT

   GLvoid            WRAPPER02V(SampleMaskSGIS, GLclampf, GLboolean)
   GLvoid            WRAPPER01V(SamplePatternSGIS, GLenum)

#endif /* GL_SGIS_multisample_OGLEXT */

// ---[ GL_SGIS_pixel_texture ]---------------------------------------------------------------------------------

#ifdef GL_SGIS_pixel_texture_OGLEXT

   GLvoid            WRAPPER02V(GetPixelTexGenParameterfvSGIS, GLenum, GLfloat *)
   GLvoid            WRAPPER02V(GetPixelTexGenParameterivSGIS, GLenum, GLint *)
   GLvoid            WRAPPER02V(PixelTexGenParameterfSGIS, GLenum, GLfloat)
   GLvoid            WRAPPER02V(PixelTexGenParameterfvSGIS, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(PixelTexGenParameteriSGIS, GLenum, GLint)
   GLvoid            WRAPPER02V(PixelTexGenParameterivSGIS, GLenum, GLint const *)

#endif /* GL_SGIS_pixel_texture_OGLEXT */

// ---[ GL_SGIS_point_parameters ]------------------------------------------------------------------------------

#ifdef GL_SGIS_point_parameters_OGLEXT

   GLvoid            WRAPPER02V(PointParameterfSGIS, GLenum, GLfloat)
   GLvoid            WRAPPER02V(PointParameterfvSGIS, GLenum, GLfloat const *)

#endif /* GL_SGIS_point_parameters_OGLEXT */

// ---[ GL_SGIS_sharpen_texture ]-------------------------------------------------------------------------------

#ifdef GL_SGIS_sharpen_texture_OGLEXT

   GLvoid            WRAPPER02V(GetSharpenTexFuncSGIS, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(SharpenTexFuncSGIS, GLenum, GLsizei, GLfloat const *)

#endif /* GL_SGIS_sharpen_texture_OGLEXT */

// ---[ GL_SGIS_texture4D ]-------------------------------------------------------------------------------------

#ifdef GL_SGIS_texture4D_OGLEXT

   GLvoid            WRAPPER11V(TexImage4DSGIS, GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *)
   GLvoid            WRAPPER13V(TexSubImage4DSGIS, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *)

#endif /* GL_SGIS_texture4D_OGLEXT */

// ---[ GL_SGIS_texture_color_mask ]----------------------------------------------------------------------------

#ifdef GL_SGIS_texture_color_mask_OGLEXT

   GLvoid            WRAPPER04V(TextureColorMaskSGIS, GLboolean, GLboolean, GLboolean, GLboolean)

#endif /* GL_SGIS_texture_color_mask_OGLEXT */

// ---[ GL_SGIS_texture_filter4 ]-------------------------------------------------------------------------------

#ifdef GL_SGIS_texture_filter4_OGLEXT

   GLvoid            WRAPPER03V(GetTexFilterFuncSGIS, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER04V(TexFilterFuncSGIS, GLenum, GLenum, GLsizei, GLfloat const *)

#endif /* GL_SGIS_texture_filter4_OGLEXT */

// ---[ GL_SGIX_async ]-----------------------------------------------------------------------------------------

#ifdef GL_SGIX_async_OGLEXT

   GLvoid            WRAPPER01V(AsyncMarkerSGIX, GLuint)
   GLvoid            WRAPPER02V(DeleteAsyncMarkersSGIX, GLuint, GLsizei)
   GLint             WRAPPER01R(FinishAsyncSGIX, GLuint *)
   GLuint            WRAPPER01R(GenAsyncMarkersSGIX, GLsizei)
   GLboolean         WRAPPER01R(IsAsyncMarkerSGIX, GLuint)
   GLint             WRAPPER01R(PollAsyncSGIX, GLuint *)

#endif /* GL_SGIX_async_OGLEXT */

// ---[ GL_SGIX_flush_raster ]----------------------------------------------------------------------------------

#ifdef GL_SGIX_flush_raster_OGLEXT

   GLvoid            WRAPPER00V(FlushRasterSGIX)

#endif /* GL_SGIX_flush_raster_OGLEXT */

// ---[ GL_SGIX_fragment_lighting ]-----------------------------------------------------------------------------

#ifdef GL_SGIX_fragment_lighting_OGLEXT

   GLvoid            WRAPPER02V(FragmentColorMaterialSGIX, GLenum, GLenum)
   GLvoid            WRAPPER03V(FragmentLightfSGIX, GLenum, GLenum, GLfloat)
   GLvoid            WRAPPER03V(FragmentLightfvSGIX, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(FragmentLightiSGIX, GLenum, GLenum, GLint)
   GLvoid            WRAPPER03V(FragmentLightivSGIX, GLenum, GLenum, GLint const *)
   GLvoid            WRAPPER02V(FragmentLightModelfSGIX, GLenum, GLfloat)
   GLvoid            WRAPPER02V(FragmentLightModelfvSGIX, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(FragmentLightModeliSGIX, GLenum, GLint)
   GLvoid            WRAPPER02V(FragmentLightModelivSGIX, GLenum, GLint const *)
   GLvoid            WRAPPER03V(FragmentMaterialfSGIX, GLenum, GLenum, GLfloat)
   GLvoid            WRAPPER03V(FragmentMaterialfvSGIX, GLenum, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(FragmentMaterialiSGIX, GLenum, GLenum, GLint)
   GLvoid            WRAPPER03V(FragmentMaterialivSGIX, GLenum, GLenum, GLint const *)
   GLvoid            WRAPPER03V(GetFragmentLightfvSGIX, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetFragmentLightivSGIX, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER03V(GetFragmentMaterialfvSGIX, GLenum, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetFragmentMaterialivSGIX, GLenum, GLenum, GLint *)
   GLvoid            WRAPPER02V(LightEnviSGIX, GLenum, GLint)

#endif /* GL_SGIX_fragment_lighting_OGLEXT */

// ---[ GL_SGIX_framezoom ]-------------------------------------------------------------------------------------

#ifdef GL_SGIX_framezoom_OGLEXT

   GLvoid            WRAPPER01V(FrameZoomSGIX, GLint)

#endif /* GL_SGIX_framezoom_OGLEXT */

// ---[ GL_SGIX_igloo_interface ]-------------------------------------------------------------------------------

#ifdef GL_SGIX_igloo_interface_OGLEXT

   GLvoid            WRAPPER02V(IglooInterfaceSGIX, GLenum, GLvoid const *)

#endif /* GL_SGIX_igloo_interface_OGLEXT */

// ---[ GL_SGIX_instruments ]-----------------------------------------------------------------------------------

#ifdef GL_SGIX_instruments_OGLEXT

   GLint             WRAPPER00R(GetInstrumentsSGIX)
   GLvoid            WRAPPER02V(InstrumentsBufferSGIX, GLsizei, GLint *)
   GLint             WRAPPER01R(PollInstrumentsSGIX, GLint *)
   GLvoid            WRAPPER01V(ReadInstrumentsSGIX, GLint)
   GLvoid            WRAPPER00V(StartInstrumentsSGIX)
   GLvoid            WRAPPER01V(StopInstrumentsSGIX, GLint)

#endif /* GL_SGIX_instruments_OGLEXT */

// ---[ GL_SGIX_list_priority ]---------------------------------------------------------------------------------

#ifdef GL_SGIX_list_priority_OGLEXT

   GLvoid            WRAPPER03V(GetListParameterfvSGIX, GLuint, GLenum, GLfloat *)
   GLvoid            WRAPPER03V(GetListParameterivSGIX, GLuint, GLenum, GLint *)
   GLvoid            WRAPPER03V(ListParameterfSGIX, GLuint, GLenum, GLfloat)
   GLvoid            WRAPPER03V(ListParameterfvSGIX, GLuint, GLenum, GLfloat const *)
   GLvoid            WRAPPER03V(ListParameteriSGIX, GLuint, GLenum, GLint)
   GLvoid            WRAPPER03V(ListParameterivSGIX, GLuint, GLenum, GLint const *)

#endif /* GL_SGIX_list_priority_OGLEXT */

// ---[ GL_SGIX_pixel_texture ]---------------------------------------------------------------------------------

#ifdef GL_SGIX_pixel_texture_OGLEXT

   GLvoid            WRAPPER01V(PixelTexGenSGIX, GLenum)

#endif /* GL_SGIX_pixel_texture_OGLEXT */

// ---[ GL_SGIX_polynomial_ffd ]--------------------------------------------------------------------------------

#ifdef GL_SGIX_polynomial_ffd_OGLEXT

   GLvoid            WRAPPER14V(DeformationMap3dSGIX, GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, GLdouble const *)
   GLvoid            WRAPPER14V(DeformationMap3fSGIX, GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat const *)
   GLvoid            WRAPPER01V(DeformSGIX, GLbitfield)
   GLvoid            WRAPPER01V(LoadIdentityDeformationMapSGIX, GLbitfield)

#endif /* GL_SGIX_polynomial_ffd_OGLEXT */

// ---[ GL_SGIX_reference_plane ]-------------------------------------------------------------------------------

#ifdef GL_SGIX_reference_plane_OGLEXT

   GLvoid            WRAPPER01V(ReferencePlaneSGIX, GLdouble const *)

#endif /* GL_SGIX_reference_plane_OGLEXT */

// ---[ GL_SGIX_sprite ]----------------------------------------------------------------------------------------

#ifdef GL_SGIX_sprite_OGLEXT

   GLvoid            WRAPPER02V(SpriteParameterfSGIX, GLenum, GLfloat)
   GLvoid            WRAPPER02V(SpriteParameterfvSGIX, GLenum, GLfloat const *)
   GLvoid            WRAPPER02V(SpriteParameteriSGIX, GLenum, GLint)
   GLvoid            WRAPPER02V(SpriteParameterivSGIX, GLenum, GLint const *)

#endif /* GL_SGIX_sprite_OGLEXT */

// ---[ GL_SGIX_tag_sample_buffer ]-----------------------------------------------------------------------------

#ifdef GL_SGIX_tag_sample_buffer_OGLEXT

   GLvoid            WRAPPER00V(TagSampleBufferSGIX)

#endif /* GL_SGIX_tag_sample_buffer_OGLEXT */

// ---[ GL_SUN_global_alpha ]-----------------------------------------------------------------------------------

#ifdef GL_SUN_global_alpha_OGLEXT

   GLvoid            WRAPPER01V(GlobalAlphaFactorbSUN, GLbyte)
   GLvoid            WRAPPER01V(GlobalAlphaFactordSUN, GLdouble)
   GLvoid            WRAPPER01V(GlobalAlphaFactorfSUN, GLfloat)
   GLvoid            WRAPPER01V(GlobalAlphaFactoriSUN, GLint)
   GLvoid            WRAPPER01V(GlobalAlphaFactorsSUN, GLshort)
   GLvoid            WRAPPER01V(GlobalAlphaFactorubSUN, GLubyte)
   GLvoid            WRAPPER01V(GlobalAlphaFactoruiSUN, GLuint)
   GLvoid            WRAPPER01V(GlobalAlphaFactorusSUN, GLushort)

#endif /* GL_SUN_global_alpha_OGLEXT */

// ---[ GL_SUN_mesh_array ]-------------------------------------------------------------------------------------

#ifdef GL_SUN_mesh_array_OGLEXT

   GLvoid            WRAPPER04V(DrawMeshArraysSUN, GLenum, GLint, GLsizei, GLsizei)

#endif /* GL_SUN_mesh_array_OGLEXT */

// ---[ GL_SUN_triangle_list ]----------------------------------------------------------------------------------

#ifdef GL_SUN_triangle_list_OGLEXT

   GLvoid            WRAPPER03V(ReplacementCodePointerSUN, GLenum, GLsizei, GLvoid const * *)
   GLvoid            WRAPPER01V(ReplacementCodeubSUN, GLubyte)
   GLvoid            WRAPPER01V(ReplacementCodeubvSUN, GLubyte const *)
   GLvoid            WRAPPER01V(ReplacementCodeuiSUN, GLuint)
   GLvoid            WRAPPER01V(ReplacementCodeuivSUN, GLuint const *)
   GLvoid            WRAPPER01V(ReplacementCodeusSUN, GLushort)
   GLvoid            WRAPPER01V(ReplacementCodeusvSUN, GLushort const *)

#endif /* GL_SUN_triangle_list_OGLEXT */

// ---[ GL_SUN_vertex ]-----------------------------------------------------------------------------------------

#ifdef GL_SUN_vertex_OGLEXT

   GLvoid            WRAPPER06V(Color3fVertex3fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(Color3fVertex3fvSUN, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER10V(Color4fNormal3fVertex3fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(Color4fNormal3fVertex3fvSUN, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER06V(Color4ubVertex2fSUN, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(Color4ubVertex2fvSUN, GLubyte const *, GLfloat const *)
   GLvoid            WRAPPER07V(Color4ubVertex3fSUN, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(Color4ubVertex3fvSUN, GLubyte const *, GLfloat const *)
   GLvoid            WRAPPER06V(Normal3fVertex3fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(Normal3fVertex3fvSUN, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER07V(ReplacementCodeuiColor3fVertex3fSUN, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(ReplacementCodeuiColor3fVertex3fvSUN, GLuint const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER11V(ReplacementCodeuiColor4fNormal3fVertex3fSUN, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER04V(ReplacementCodeuiColor4fNormal3fVertex3fvSUN, GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER08V(ReplacementCodeuiColor4ubVertex3fSUN, GLuint, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(ReplacementCodeuiColor4ubVertex3fvSUN, GLuint const *, GLubyte const *, GLfloat const *)
   GLvoid            WRAPPER07V(ReplacementCodeuiNormal3fVertex3fSUN, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(ReplacementCodeuiNormal3fVertex3fvSUN, GLuint const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER13V(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER05V(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN, GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER09V(ReplacementCodeuiTexCoord2fNormal3fVertex3fSUN, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER04V(ReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN, GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER06V(ReplacementCodeuiTexCoord2fVertex3fSUN, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(ReplacementCodeuiTexCoord2fVertex3fvSUN, GLuint const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER04V(ReplacementCodeuiVertex3fSUN, GLuint, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(ReplacementCodeuiVertex3fvSUN, GLuint const *, GLfloat const *)
   GLvoid            WRAPPER08V(TexCoord2fColor3fVertex3fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(TexCoord2fColor3fVertex3fvSUN, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER12V(TexCoord2fColor4fNormal3fVertex3fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER04V(TexCoord2fColor4fNormal3fVertex3fvSUN, GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER09V(TexCoord2fColor4ubVertex3fSUN, GLfloat, GLfloat, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(TexCoord2fColor4ubVertex3fvSUN, GLfloat const *, GLubyte const *, GLfloat const *)
   GLvoid            WRAPPER08V(TexCoord2fNormal3fVertex3fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER03V(TexCoord2fNormal3fVertex3fvSUN, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER05V(TexCoord2fVertex3fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(TexCoord2fVertex3fvSUN, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER15V(TexCoord4fColor4fNormal3fVertex4fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER04V(TexCoord4fColor4fNormal3fVertex4fvSUN, GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *)
   GLvoid            WRAPPER08V(TexCoord4fVertex4fSUN, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)
   GLvoid            WRAPPER02V(TexCoord4fVertex4fvSUN, GLfloat const *, GLfloat const *)

#endif /* GL_SUN_vertex_OGLEXT */

// ---[ GL_SUNX_constant_data ]---------------------------------------------------------------------------------

#ifdef GL_SUNX_constant_data_OGLEXT

   GLvoid            WRAPPER00V(FinishTextureSUNX)

#endif /* GL_SUNX_constant_data_OGLEXT */

