/* glext.h                                                Copyright (C) 2004 Thomas Jansen (jansen@caesar.de) */
/*                                                                  (C) 2004 research center caesar           */
/*                                                                                                            */
/* This file is part of OglExt, a free OpenGL extension library.                                              */
/*                                                                                                            */
/* This program is free software; you can redistribute it and/or modify it under the terms of the GNU  Lesser */
/* General Public License as published by the Free Software Foundation; either version 2.1 of the License, or */
/* (at your option) any later version.                                                                        */
/*                                                                                                            */
/* This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the */
/* implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE.   See  the  GNU  Lesser  General */
/* Public License for more details.                                                                           */
/*                                                                                                            */
/* You should have received a copy of the GNU Lesser General Public License along with this library; if  not, */
/* write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA       */
/*                                                                                                            */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
/*                                                                                                            */
/* This file was automatically generated on June 15, 2004, 11:47 am                                           */

#ifndef _GLEXT_H_
#define _GLEXT_H_

#ifdef __cplusplus
   extern "C" {
#endif


/* ========================================================================================================== */
/* ===                                    P R E - D E F I N I T I O N S                                   === */
/* ========================================================================================================== */

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
	#define	WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
#endif

#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
	#include <gl.h>
#else
	#include <GL/gl.h>
#endif

#if !defined(APIENTRY)
   #define APIENTRY
#endif

#if !defined(GLAPI)
   #if defined(_WIN32)
      #define GLAPI extern _declspec(dllimport)
   #else
      #define GLAPI extern
   #endif
#endif

#include <stddef.h>


/* ========================================================================================================== */
/* ===                                             M A C R O S                                            === */
/* ========================================================================================================== */

#ifndef	OGLGLEXT_NOALIAS

	/* Macro for creating the OpenGL name (change the alias prefix if needed) */

	#ifndef	OGLEXT_MAKEGLNAME
		#define	OGLEXT_MAKEGLNAME(NAME)		oglext ## NAME
	#endif	/* OGLEXT_MAKEGLNAME */

#endif	/* OGLGLEXT_NOALIAS */


/* ========================================================================================================== */
/* ===                                    O P E N G L   V E R S I O N                                     === */
/* ========================================================================================================== */

/* ---[ GL_VERSION_1_2 ]------------------------------------------------------------------------------------- */

/*! link : http://www.opengl.org/documentation/specs/version1.2/opengl1.2.1.pdf */

#ifndef GL_VERSION_1_2

   #define GL_VERSION_1_2 1
   #define GL_VERSION_1_2_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SMOOTH_POINT_SIZE_RANGE

      #define GL_SMOOTH_POINT_SIZE_RANGE                                  0xb12
      #define GL_SMOOTH_POINT_SIZE_GRANULARITY                            0xb13
      #define GL_SMOOTH_LINE_WIDTH_RANGE                                  0xb22
      #define GL_SMOOTH_LINE_WIDTH_GRANULARITY                            0xb23
      #define GL_UNSIGNED_BYTE_3_3_2                                      0x8032
      #define GL_UNSIGNED_SHORT_4_4_4_4                                   0x8033
      #define GL_UNSIGNED_SHORT_5_5_5_1                                   0x8034
      #define GL_UNSIGNED_INT_8_8_8_8                                     0x8035
      #define GL_UNSIGNED_INT_10_10_10_2                                  0x8036
      #define GL_RESCALE_NORMAL                                           0x803a
      #define GL_TEXTURE_BINDING_3D                                       0x806a
      #define GL_PACK_SKIP_IMAGES                                         0x806b
      #define GL_PACK_IMAGE_HEIGHT                                        0x806c
      #define GL_UNPACK_SKIP_IMAGES                                       0x806d
      #define GL_UNPACK_IMAGE_HEIGHT                                      0x806e
      #define GL_TEXTURE_3D                                               0x806f
      #define GL_PROXY_TEXTURE_3D                                         0x8070
      #define GL_TEXTURE_DEPTH                                            0x8071
      #define GL_TEXTURE_WRAP_R                                           0x8072
      #define GL_MAX_3D_TEXTURE_SIZE                                      0x8073
      #define GL_BGR                                                      0x80e0
      #define GL_BGRA                                                     0x80e1
      #define GL_MAX_ELEMENTS_VERTICES                                    0x80e8
      #define GL_MAX_ELEMENTS_INDICES                                     0x80e9
      #define GL_CLAMP_TO_EDGE                                            0x812f
      #define GL_TEXTURE_MIN_LOD                                          0x813a
      #define GL_TEXTURE_MAX_LOD                                          0x813b
      #define GL_TEXTURE_BASE_LEVEL                                       0x813c
      #define GL_TEXTURE_MAX_LEVEL                                        0x813d
      #define GL_LIGHT_MODEL_COLOR_CONTROL                                0x81f8
      #define GL_SINGLE_COLOR                                             0x81f9
      #define GL_SEPARATE_SPECULAR_COLOR                                  0x81fa
      #define GL_UNSIGNED_BYTE_2_3_3_REV                                  0x8362
      #define GL_UNSIGNED_SHORT_5_6_5                                     0x8363
      #define GL_UNSIGNED_SHORT_5_6_5_REV                                 0x8364
      #define GL_UNSIGNED_SHORT_4_4_4_4_REV                               0x8365
      #define GL_UNSIGNED_SHORT_1_5_5_5_REV                               0x8366
      #define GL_UNSIGNED_INT_8_8_8_8_REV                                 0x8367
      #define GL_UNSIGNED_INT_2_10_10_10_REV                              0x8368
      #define GL_ALIASED_POINT_SIZE_RANGE                                 0x846d
      #define GL_ALIASED_LINE_WIDTH_RANGE                                 0x846e

   #endif /* GL_SMOOTH_POINT_SIZE_RANGE */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBlendColor                                     OGLEXT_MAKEGLNAME(BlendColor)
      #define glBlendEquation                                  OGLEXT_MAKEGLNAME(BlendEquation)
      #define glColorSubTable                                  OGLEXT_MAKEGLNAME(ColorSubTable)
      #define glColorTable                                     OGLEXT_MAKEGLNAME(ColorTable)
      #define glColorTableParameterfv                          OGLEXT_MAKEGLNAME(ColorTableParameterfv)
      #define glColorTableParameteriv                          OGLEXT_MAKEGLNAME(ColorTableParameteriv)
      #define glConvolutionFilter1D                            OGLEXT_MAKEGLNAME(ConvolutionFilter1D)
      #define glConvolutionFilter2D                            OGLEXT_MAKEGLNAME(ConvolutionFilter2D)
      #define glConvolutionParameterf                          OGLEXT_MAKEGLNAME(ConvolutionParameterf)
      #define glConvolutionParameterfv                         OGLEXT_MAKEGLNAME(ConvolutionParameterfv)
      #define glConvolutionParameteri                          OGLEXT_MAKEGLNAME(ConvolutionParameteri)
      #define glConvolutionParameteriv                         OGLEXT_MAKEGLNAME(ConvolutionParameteriv)
      #define glCopyColorSubTable                              OGLEXT_MAKEGLNAME(CopyColorSubTable)
      #define glCopyColorTable                                 OGLEXT_MAKEGLNAME(CopyColorTable)
      #define glCopyConvolutionFilter1D                        OGLEXT_MAKEGLNAME(CopyConvolutionFilter1D)
      #define glCopyConvolutionFilter2D                        OGLEXT_MAKEGLNAME(CopyConvolutionFilter2D)
      #define glCopyTexSubImage3D                              OGLEXT_MAKEGLNAME(CopyTexSubImage3D)
      #define glDrawRangeElements                              OGLEXT_MAKEGLNAME(DrawRangeElements)
      #define glGetColorTable                                  OGLEXT_MAKEGLNAME(GetColorTable)
      #define glGetColorTableParameterfv                       OGLEXT_MAKEGLNAME(GetColorTableParameterfv)
      #define glGetColorTableParameteriv                       OGLEXT_MAKEGLNAME(GetColorTableParameteriv)
      #define glGetConvolutionFilter                           OGLEXT_MAKEGLNAME(GetConvolutionFilter)
      #define glGetConvolutionParameterfv                      OGLEXT_MAKEGLNAME(GetConvolutionParameterfv)
      #define glGetConvolutionParameteriv                      OGLEXT_MAKEGLNAME(GetConvolutionParameteriv)
      #define glGetHistogram                                   OGLEXT_MAKEGLNAME(GetHistogram)
      #define glGetHistogramParameterfv                        OGLEXT_MAKEGLNAME(GetHistogramParameterfv)
      #define glGetHistogramParameteriv                        OGLEXT_MAKEGLNAME(GetHistogramParameteriv)
      #define glGetMinmax                                      OGLEXT_MAKEGLNAME(GetMinmax)
      #define glGetMinmaxParameterfv                           OGLEXT_MAKEGLNAME(GetMinmaxParameterfv)
      #define glGetMinmaxParameteriv                           OGLEXT_MAKEGLNAME(GetMinmaxParameteriv)
      #define glGetSeparableFilter                             OGLEXT_MAKEGLNAME(GetSeparableFilter)
      #define glHistogram                                      OGLEXT_MAKEGLNAME(Histogram)
      #define glMinmax                                         OGLEXT_MAKEGLNAME(Minmax)
      #define glResetHistogram                                 OGLEXT_MAKEGLNAME(ResetHistogram)
      #define glResetMinmax                                    OGLEXT_MAKEGLNAME(ResetMinmax)
      #define glSeparableFilter2D                              OGLEXT_MAKEGLNAME(SeparableFilter2D)
      #define glTexImage3D                                     OGLEXT_MAKEGLNAME(TexImage3D)
      #define glTexSubImage3D                                  OGLEXT_MAKEGLNAME(TexSubImage3D)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBlendColor(GLclampf, GLclampf, GLclampf, GLclampf);
      GLAPI GLvoid            glBlendEquation(GLenum);
      GLAPI GLvoid            glColorSubTable(GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glColorTable(GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glColorTableParameterfv(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glColorTableParameteriv(GLenum, GLenum, GLint const *);
      GLAPI GLvoid            glConvolutionFilter1D(GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glConvolutionFilter2D(GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glConvolutionParameterf(GLenum, GLenum, GLfloat);
      GLAPI GLvoid            glConvolutionParameterfv(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glConvolutionParameteri(GLenum, GLenum, GLint);
      GLAPI GLvoid            glConvolutionParameteriv(GLenum, GLenum, GLint const *);
      GLAPI GLvoid            glCopyColorSubTable(GLenum, GLsizei, GLint, GLint, GLsizei);
      GLAPI GLvoid            glCopyColorTable(GLenum, GLenum, GLint, GLint, GLsizei);
      GLAPI GLvoid            glCopyConvolutionFilter1D(GLenum, GLenum, GLint, GLint, GLsizei);
      GLAPI GLvoid            glCopyConvolutionFilter2D(GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
      GLAPI GLvoid            glCopyTexSubImage3D(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
      GLAPI GLvoid            glDrawRangeElements(GLenum, GLuint, GLuint, GLsizei, GLenum, GLvoid const *);
      GLAPI GLvoid            glGetColorTable(GLenum, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetColorTableParameterfv(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetColorTableParameteriv(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetConvolutionFilter(GLenum, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetConvolutionParameterfv(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetConvolutionParameteriv(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetHistogram(GLenum, GLboolean, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetHistogramParameterfv(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetHistogramParameteriv(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetMinmax(GLenum, GLboolean, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetMinmaxParameterfv(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetMinmaxParameteriv(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetSeparableFilter(GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
      GLAPI GLvoid            glHistogram(GLenum, GLsizei, GLenum, GLboolean);
      GLAPI GLvoid            glMinmax(GLenum, GLenum, GLboolean);
      GLAPI GLvoid            glResetHistogram(GLenum);
      GLAPI GLvoid            glResetMinmax(GLenum);
      GLAPI GLvoid            glSeparableFilter2D(GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *, GLvoid const *);
      GLAPI GLvoid            glTexImage3D(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glTexSubImage3D(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_VERSION_1_2 */


/* ---[ GL_VERSION_1_3 ]------------------------------------------------------------------------------------- */

/*! link : http://www.opengl.org/documentation/specs/version1.3/glspec13.pdf */

#ifndef GL_VERSION_1_3

   #define GL_VERSION_1_3 1
   #define GL_VERSION_1_3_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MULTISAMPLE

      #define GL_MULTISAMPLE                                              0x809d
      #define GL_SAMPLE_ALPHA_TO_COVERAGE                                 0x809e
      #define GL_SAMPLE_ALPHA_TO_ONE                                      0x809f
      #define GL_SAMPLE_COVERAGE                                          0x80a0
      #define GL_SAMPLE_BUFFERS                                           0x80a8
      #define GL_SAMPLES                                                  0x80a9
      #define GL_SAMPLE_COVERAGE_VALUE                                    0x80aa
      #define GL_SAMPLE_COVERAGE_INVERT                                   0x80ab
      #define GL_CLAMP_TO_BORDER                                          0x812d
      #define GL_TEXTURE0                                                 0x84c0
      #define GL_TEXTURE1                                                 0x84c1
      #define GL_TEXTURE2                                                 0x84c2
      #define GL_TEXTURE3                                                 0x84c3
      #define GL_TEXTURE4                                                 0x84c4
      #define GL_TEXTURE5                                                 0x84c5
      #define GL_TEXTURE6                                                 0x84c6
      #define GL_TEXTURE7                                                 0x84c7
      #define GL_TEXTURE8                                                 0x84c8
      #define GL_TEXTURE9                                                 0x84c9
      #define GL_TEXTURE10                                                0x84ca
      #define GL_TEXTURE11                                                0x84cb
      #define GL_TEXTURE12                                                0x84cc
      #define GL_TEXTURE13                                                0x84cd
      #define GL_TEXTURE14                                                0x84ce
      #define GL_TEXTURE15                                                0x84cf
      #define GL_TEXTURE16                                                0x84d0
      #define GL_TEXTURE17                                                0x84d1
      #define GL_TEXTURE18                                                0x84d2
      #define GL_TEXTURE19                                                0x84d3
      #define GL_TEXTURE20                                                0x84d4
      #define GL_TEXTURE21                                                0x84d5
      #define GL_TEXTURE22                                                0x84d6
      #define GL_TEXTURE23                                                0x84d7
      #define GL_TEXTURE24                                                0x84d8
      #define GL_TEXTURE25                                                0x84d9
      #define GL_TEXTURE26                                                0x84da
      #define GL_TEXTURE27                                                0x84db
      #define GL_TEXTURE28                                                0x84dc
      #define GL_TEXTURE29                                                0x84dd
      #define GL_TEXTURE30                                                0x84de
      #define GL_TEXTURE31                                                0x84df
      #define GL_ACTIVE_TEXTURE                                           0x84e0
      #define GL_CLIENT_ACTIVE_TEXTURE                                    0x84e1
      #define GL_MAX_TEXTURE_UNITS                                        0x84e2
      #define GL_TRANSPOSE_MODELVIEW_MATRIX                               0x84e3
      #define GL_TRANSPOSE_PROJECTION_MATRIX                              0x84e4
      #define GL_TRANSPOSE_TEXTURE_MATRIX                                 0x84e5
      #define GL_TRANSPOSE_COLOR_MATRIX                                   0x84e6
      #define GL_SUBTRACT                                                 0x84e7
      #define GL_COMPRESSED_ALPHA                                         0x84e9
      #define GL_COMPRESSED_LUMINANCE                                     0x84ea
      #define GL_COMPRESSED_LUMINANCE_ALPHA                               0x84eb
      #define GL_COMPRESSED_INTENSITY                                     0x84ec
      #define GL_COMPRESSED_RGB                                           0x84ed
      #define GL_COMPRESSED_RGBA                                          0x84ee
      #define GL_TEXTURE_COMPRESSION_HINT                                 0x84ef
      #define GL_NORMAL_MAP                                               0x8511
      #define GL_REFLECTION_MAP                                           0x8512
      #define GL_TEXTURE_CUBE_MAP                                         0x8513
      #define GL_TEXTURE_BINDING_CUBE_MAP                                 0x8514
      #define GL_TEXTURE_CUBE_MAP_POSITIVE_X                              0x8515
      #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                              0x8516
      #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                              0x8517
      #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                              0x8518
      #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                              0x8519
      #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                              0x851a
      #define GL_PROXY_TEXTURE_CUBE_MAP                                   0x851b
      #define GL_MAX_CUBE_MAP_TEXTURE_SIZE                                0x851c
      #define GL_COMBINE                                                  0x8570
      #define GL_COMBINE_RGB                                              0x8571
      #define GL_COMBINE_ALPHA                                            0x8572
      #define GL_RGB_SCALE                                                0x8573
      #define GL_ADD_SIGNED                                               0x8574
      #define GL_INTERPOLATE                                              0x8575
      #define GL_CONSTANT                                                 0x8576
      #define GL_PRIMARY_COLOR                                            0x8577
      #define GL_PREVIOUS                                                 0x8578
      #define GL_SOURCE0_RGB                                              0x8580
      #define GL_SOURCE1_RGB                                              0x8581
      #define GL_SOURCE2_RGB                                              0x8582
      #define GL_SOURCE0_ALPHA                                            0x8588
      #define GL_SOURCE1_ALPHA                                            0x8589
      #define GL_SOURCE2_ALPHA                                            0x858a
      #define GL_OPERAND0_RGB                                             0x8590
      #define GL_OPERAND1_RGB                                             0x8591
      #define GL_OPERAND2_RGB                                             0x8592
      #define GL_OPERAND0_ALPHA                                           0x8598
      #define GL_OPERAND1_ALPHA                                           0x8599
      #define GL_OPERAND2_ALPHA                                           0x859a
      #define GL_TEXTURE_COMPRESSED_IMAGE_SIZE                            0x86a0
      #define GL_TEXTURE_COMPRESSED                                       0x86a1
      #define GL_NUM_COMPRESSED_TEXTURE_FORMATS                           0x86a2
      #define GL_COMPRESSED_TEXTURE_FORMATS                               0x86a3
      #define GL_DOT3_RGB                                                 0x86ae
      #define GL_DOT3_RGBA                                                0x86af
      #define GL_MULTISAMPLE_BIT                                          0x20000000

   #endif /* GL_MULTISAMPLE */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glActiveTexture                                  OGLEXT_MAKEGLNAME(ActiveTexture)
      #define glClientActiveTexture                            OGLEXT_MAKEGLNAME(ClientActiveTexture)
      #define glCompressedTexImage1D                           OGLEXT_MAKEGLNAME(CompressedTexImage1D)
      #define glCompressedTexImage2D                           OGLEXT_MAKEGLNAME(CompressedTexImage2D)
      #define glCompressedTexImage3D                           OGLEXT_MAKEGLNAME(CompressedTexImage3D)
      #define glCompressedTexSubImage1D                        OGLEXT_MAKEGLNAME(CompressedTexSubImage1D)
      #define glCompressedTexSubImage2D                        OGLEXT_MAKEGLNAME(CompressedTexSubImage2D)
      #define glCompressedTexSubImage3D                        OGLEXT_MAKEGLNAME(CompressedTexSubImage3D)
      #define glGetCompressedTexImage                          OGLEXT_MAKEGLNAME(GetCompressedTexImage)
      #define glLoadTransposeMatrixd                           OGLEXT_MAKEGLNAME(LoadTransposeMatrixd)
      #define glLoadTransposeMatrixf                           OGLEXT_MAKEGLNAME(LoadTransposeMatrixf)
      #define glMultiTexCoord1d                                OGLEXT_MAKEGLNAME(MultiTexCoord1d)
      #define glMultiTexCoord1dv                               OGLEXT_MAKEGLNAME(MultiTexCoord1dv)
      #define glMultiTexCoord1f                                OGLEXT_MAKEGLNAME(MultiTexCoord1f)
      #define glMultiTexCoord1fv                               OGLEXT_MAKEGLNAME(MultiTexCoord1fv)
      #define glMultiTexCoord1i                                OGLEXT_MAKEGLNAME(MultiTexCoord1i)
      #define glMultiTexCoord1iv                               OGLEXT_MAKEGLNAME(MultiTexCoord1iv)
      #define glMultiTexCoord1s                                OGLEXT_MAKEGLNAME(MultiTexCoord1s)
      #define glMultiTexCoord1sv                               OGLEXT_MAKEGLNAME(MultiTexCoord1sv)
      #define glMultiTexCoord2d                                OGLEXT_MAKEGLNAME(MultiTexCoord2d)
      #define glMultiTexCoord2dv                               OGLEXT_MAKEGLNAME(MultiTexCoord2dv)
      #define glMultiTexCoord2f                                OGLEXT_MAKEGLNAME(MultiTexCoord2f)
      #define glMultiTexCoord2fv                               OGLEXT_MAKEGLNAME(MultiTexCoord2fv)
      #define glMultiTexCoord2i                                OGLEXT_MAKEGLNAME(MultiTexCoord2i)
      #define glMultiTexCoord2iv                               OGLEXT_MAKEGLNAME(MultiTexCoord2iv)
      #define glMultiTexCoord2s                                OGLEXT_MAKEGLNAME(MultiTexCoord2s)
      #define glMultiTexCoord2sv                               OGLEXT_MAKEGLNAME(MultiTexCoord2sv)
      #define glMultiTexCoord3d                                OGLEXT_MAKEGLNAME(MultiTexCoord3d)
      #define glMultiTexCoord3dv                               OGLEXT_MAKEGLNAME(MultiTexCoord3dv)
      #define glMultiTexCoord3f                                OGLEXT_MAKEGLNAME(MultiTexCoord3f)
      #define glMultiTexCoord3fv                               OGLEXT_MAKEGLNAME(MultiTexCoord3fv)
      #define glMultiTexCoord3i                                OGLEXT_MAKEGLNAME(MultiTexCoord3i)
      #define glMultiTexCoord3iv                               OGLEXT_MAKEGLNAME(MultiTexCoord3iv)
      #define glMultiTexCoord3s                                OGLEXT_MAKEGLNAME(MultiTexCoord3s)
      #define glMultiTexCoord3sv                               OGLEXT_MAKEGLNAME(MultiTexCoord3sv)
      #define glMultiTexCoord4d                                OGLEXT_MAKEGLNAME(MultiTexCoord4d)
      #define glMultiTexCoord4dv                               OGLEXT_MAKEGLNAME(MultiTexCoord4dv)
      #define glMultiTexCoord4f                                OGLEXT_MAKEGLNAME(MultiTexCoord4f)
      #define glMultiTexCoord4fv                               OGLEXT_MAKEGLNAME(MultiTexCoord4fv)
      #define glMultiTexCoord4i                                OGLEXT_MAKEGLNAME(MultiTexCoord4i)
      #define glMultiTexCoord4iv                               OGLEXT_MAKEGLNAME(MultiTexCoord4iv)
      #define glMultiTexCoord4s                                OGLEXT_MAKEGLNAME(MultiTexCoord4s)
      #define glMultiTexCoord4sv                               OGLEXT_MAKEGLNAME(MultiTexCoord4sv)
      #define glMultTransposeMatrixd                           OGLEXT_MAKEGLNAME(MultTransposeMatrixd)
      #define glMultTransposeMatrixf                           OGLEXT_MAKEGLNAME(MultTransposeMatrixf)
      #define glSampleCoverage                                 OGLEXT_MAKEGLNAME(SampleCoverage)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glActiveTexture(GLenum);
      GLAPI GLvoid            glClientActiveTexture(GLenum);
      GLAPI GLvoid            glCompressedTexImage1D(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexImage2D(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexImage3D(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexSubImage1D(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexSubImage3D(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glGetCompressedTexImage(GLenum, GLint, GLvoid *);
      GLAPI GLvoid            glLoadTransposeMatrixd(GLdouble const *);
      GLAPI GLvoid            glLoadTransposeMatrixf(GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord1d(GLenum, GLdouble);
      GLAPI GLvoid            glMultiTexCoord1dv(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord1f(GLenum, GLfloat);
      GLAPI GLvoid            glMultiTexCoord1fv(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord1i(GLenum, GLint);
      GLAPI GLvoid            glMultiTexCoord1iv(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord1s(GLenum, GLshort);
      GLAPI GLvoid            glMultiTexCoord1sv(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord2d(GLenum, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord2dv(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord2f(GLenum, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord2fv(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord2i(GLenum, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord2iv(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord2s(GLenum, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord2sv(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord3d(GLenum, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord3dv(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord3f(GLenum, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord3fv(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord3i(GLenum, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord3iv(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord3s(GLenum, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord3sv(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord4d(GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord4dv(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord4f(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord4fv(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord4i(GLenum, GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord4iv(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord4s(GLenum, GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord4sv(GLenum, GLshort const *);
      GLAPI GLvoid            glMultTransposeMatrixd(GLdouble const *);
      GLAPI GLvoid            glMultTransposeMatrixf(GLfloat const *);
      GLAPI GLvoid            glSampleCoverage(GLclampf, GLboolean);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_VERSION_1_3 */


/* ---[ GL_VERSION_1_4 ]------------------------------------------------------------------------------------- */

/*! link : http://www.opengl.org/documentation/specs/version1.4/glspec14.pdf */

#ifndef GL_VERSION_1_4

   #define GL_VERSION_1_4 1
   #define GL_VERSION_1_4_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_BLEND_DST_RGB

      #define GL_BLEND_DST_RGB                                            0x80c8
      #define GL_BLEND_SRC_RGB                                            0x80c9
      #define GL_BLEND_DST_ALPHA                                          0x80ca
      #define GL_BLEND_SRC_ALPHA                                          0x80cb
      #define GL_POINT_SIZE_MIN                                           0x8126
      #define GL_POINT_SIZE_MAX                                           0x8127
      #define GL_POINT_FADE_THRESHOLD_SIZE                                0x8128
      #define GL_POINT_DISTANCE_ATTENUATION                               0x8129
      #define GL_GENERATE_MIPMAP                                          0x8191
      #define GL_GENERATE_MIPMAP_HINT                                     0x8192
      #define GL_DEPTH_COMPONENT16                                        0x81a5
      #define GL_DEPTH_COMPONENT24                                        0x81a6
      #define GL_DEPTH_COMPONENT32                                        0x81a7
      #define GL_MIRRORED_REPEAT                                          0x8370
      #define GL_FOG_COORDINATE_SOURCE                                    0x8450
      #define GL_FOG_COORDINATE                                           0x8451
      #define GL_FRAGMENT_DEPTH                                           0x8452
      #define GL_CURRENT_FOG_COORDINATE                                   0x8453
      #define GL_FOG_COORDINATE_ARRAY_TYPE                                0x8454
      #define GL_FOG_COORDINATE_ARRAY_STRIDE                              0x8455
      #define GL_FOG_COORDINATE_ARRAY_POINTER                             0x8456
      #define GL_FOG_COORDINATE_ARRAY                                     0x8457
      #define GL_COLOR_SUM                                                0x8458
      #define GL_CURRENT_SECONDARY_COLOR                                  0x8459
      #define GL_SECONDARY_COLOR_ARRAY_SIZE                               0x845a
      #define GL_SECONDARY_COLOR_ARRAY_TYPE                               0x845b
      #define GL_SECONDARY_COLOR_ARRAY_STRIDE                             0x845c
      #define GL_SECONDARY_COLOR_ARRAY_POINTER                            0x845d
      #define GL_SECONDARY_COLOR_ARRAY                                    0x845e
      #define GL_MAX_TEXTURE_LOD_BIAS                                     0x84fd
      #define GL_TEXTURE_FILTER_CONTROL                                   0x8500
      #define GL_TEXTURE_LOD_BIAS                                         0x8501
      #define GL_INCR_WRAP                                                0x8507
      #define GL_DECR_WRAP                                                0x8508
      #define GL_TEXTURE_DEPTH_SIZE                                       0x884a
      #define GL_DEPTH_TEXTURE_MODE                                       0x884b
      #define GL_TEXTURE_COMPARE_MODE                                     0x884c
      #define GL_TEXTURE_COMPARE_FUNC                                     0x884d
      #define GL_COMPARE_R_TO_TEXTURE                                     0x884e

   #endif /* GL_BLEND_DST_RGB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBlendFuncSeparate                              OGLEXT_MAKEGLNAME(BlendFuncSeparate)
      #define glFogCoordd                                      OGLEXT_MAKEGLNAME(FogCoordd)
      #define glFogCoorddv                                     OGLEXT_MAKEGLNAME(FogCoorddv)
      #define glFogCoordf                                      OGLEXT_MAKEGLNAME(FogCoordf)
      #define glFogCoordfv                                     OGLEXT_MAKEGLNAME(FogCoordfv)
      #define glFogCoordPointer                                OGLEXT_MAKEGLNAME(FogCoordPointer)
      #define glMultiDrawArrays                                OGLEXT_MAKEGLNAME(MultiDrawArrays)
      #define glMultiDrawElements                              OGLEXT_MAKEGLNAME(MultiDrawElements)
      #define glPointParameterf                                OGLEXT_MAKEGLNAME(PointParameterf)
      #define glPointParameterfv                               OGLEXT_MAKEGLNAME(PointParameterfv)
      #define glPointParameteri                                OGLEXT_MAKEGLNAME(PointParameteri)
      #define glPointParameteriv                               OGLEXT_MAKEGLNAME(PointParameteriv)
      #define glSecondaryColor3b                               OGLEXT_MAKEGLNAME(SecondaryColor3b)
      #define glSecondaryColor3bv                              OGLEXT_MAKEGLNAME(SecondaryColor3bv)
      #define glSecondaryColor3d                               OGLEXT_MAKEGLNAME(SecondaryColor3d)
      #define glSecondaryColor3dv                              OGLEXT_MAKEGLNAME(SecondaryColor3dv)
      #define glSecondaryColor3f                               OGLEXT_MAKEGLNAME(SecondaryColor3f)
      #define glSecondaryColor3fv                              OGLEXT_MAKEGLNAME(SecondaryColor3fv)
      #define glSecondaryColor3i                               OGLEXT_MAKEGLNAME(SecondaryColor3i)
      #define glSecondaryColor3iv                              OGLEXT_MAKEGLNAME(SecondaryColor3iv)
      #define glSecondaryColor3s                               OGLEXT_MAKEGLNAME(SecondaryColor3s)
      #define glSecondaryColor3sv                              OGLEXT_MAKEGLNAME(SecondaryColor3sv)
      #define glSecondaryColor3ub                              OGLEXT_MAKEGLNAME(SecondaryColor3ub)
      #define glSecondaryColor3ubv                             OGLEXT_MAKEGLNAME(SecondaryColor3ubv)
      #define glSecondaryColor3ui                              OGLEXT_MAKEGLNAME(SecondaryColor3ui)
      #define glSecondaryColor3uiv                             OGLEXT_MAKEGLNAME(SecondaryColor3uiv)
      #define glSecondaryColor3us                              OGLEXT_MAKEGLNAME(SecondaryColor3us)
      #define glSecondaryColor3usv                             OGLEXT_MAKEGLNAME(SecondaryColor3usv)
      #define glSecondaryColorPointer                          OGLEXT_MAKEGLNAME(SecondaryColorPointer)
      #define glWindowPos2d                                    OGLEXT_MAKEGLNAME(WindowPos2d)
      #define glWindowPos2dv                                   OGLEXT_MAKEGLNAME(WindowPos2dv)
      #define glWindowPos2f                                    OGLEXT_MAKEGLNAME(WindowPos2f)
      #define glWindowPos2fv                                   OGLEXT_MAKEGLNAME(WindowPos2fv)
      #define glWindowPos2i                                    OGLEXT_MAKEGLNAME(WindowPos2i)
      #define glWindowPos2iv                                   OGLEXT_MAKEGLNAME(WindowPos2iv)
      #define glWindowPos2s                                    OGLEXT_MAKEGLNAME(WindowPos2s)
      #define glWindowPos2sv                                   OGLEXT_MAKEGLNAME(WindowPos2sv)
      #define glWindowPos3d                                    OGLEXT_MAKEGLNAME(WindowPos3d)
      #define glWindowPos3dv                                   OGLEXT_MAKEGLNAME(WindowPos3dv)
      #define glWindowPos3f                                    OGLEXT_MAKEGLNAME(WindowPos3f)
      #define glWindowPos3fv                                   OGLEXT_MAKEGLNAME(WindowPos3fv)
      #define glWindowPos3i                                    OGLEXT_MAKEGLNAME(WindowPos3i)
      #define glWindowPos3iv                                   OGLEXT_MAKEGLNAME(WindowPos3iv)
      #define glWindowPos3s                                    OGLEXT_MAKEGLNAME(WindowPos3s)
      #define glWindowPos3sv                                   OGLEXT_MAKEGLNAME(WindowPos3sv)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBlendFuncSeparate(GLenum, GLenum, GLenum, GLenum);
      GLAPI GLvoid            glFogCoordd(GLdouble);
      GLAPI GLvoid            glFogCoorddv(GLdouble const *);
      GLAPI GLvoid            glFogCoordf(GLfloat);
      GLAPI GLvoid            glFogCoordfv(GLfloat const *);
      GLAPI GLvoid            glFogCoordPointer(GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glMultiDrawArrays(GLenum, GLint *, GLsizei *, GLsizei);
      GLAPI GLvoid            glMultiDrawElements(GLenum, GLsizei const *, GLenum, GLvoid const * *, GLsizei);
      GLAPI GLvoid            glPointParameterf(GLenum, GLfloat);
      GLAPI GLvoid            glPointParameterfv(GLenum, GLfloat const *);
      GLAPI GLvoid            glPointParameteri(GLenum, GLint);
      GLAPI GLvoid            glPointParameteriv(GLenum, GLint const *);
      GLAPI GLvoid            glSecondaryColor3b(GLbyte, GLbyte, GLbyte);
      GLAPI GLvoid            glSecondaryColor3bv(GLbyte const *);
      GLAPI GLvoid            glSecondaryColor3d(GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glSecondaryColor3dv(GLdouble const *);
      GLAPI GLvoid            glSecondaryColor3f(GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glSecondaryColor3fv(GLfloat const *);
      GLAPI GLvoid            glSecondaryColor3i(GLint, GLint, GLint);
      GLAPI GLvoid            glSecondaryColor3iv(GLint const *);
      GLAPI GLvoid            glSecondaryColor3s(GLshort, GLshort, GLshort);
      GLAPI GLvoid            glSecondaryColor3sv(GLshort const *);
      GLAPI GLvoid            glSecondaryColor3ub(GLubyte, GLubyte, GLubyte);
      GLAPI GLvoid            glSecondaryColor3ubv(GLubyte const *);
      GLAPI GLvoid            glSecondaryColor3ui(GLuint, GLuint, GLuint);
      GLAPI GLvoid            glSecondaryColor3uiv(GLuint const *);
      GLAPI GLvoid            glSecondaryColor3us(GLushort, GLushort, GLushort);
      GLAPI GLvoid            glSecondaryColor3usv(GLushort const *);
      GLAPI GLvoid            glSecondaryColorPointer(GLint, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glWindowPos2d(GLdouble, GLdouble);
      GLAPI GLvoid            glWindowPos2dv(GLdouble const *);
      GLAPI GLvoid            glWindowPos2f(GLfloat, GLfloat);
      GLAPI GLvoid            glWindowPos2fv(GLfloat const *);
      GLAPI GLvoid            glWindowPos2i(GLint, GLint);
      GLAPI GLvoid            glWindowPos2iv(GLint const *);
      GLAPI GLvoid            glWindowPos2s(GLshort, GLshort);
      GLAPI GLvoid            glWindowPos2sv(GLshort const *);
      GLAPI GLvoid            glWindowPos3d(GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glWindowPos3dv(GLdouble const *);
      GLAPI GLvoid            glWindowPos3f(GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glWindowPos3fv(GLfloat const *);
      GLAPI GLvoid            glWindowPos3i(GLint, GLint, GLint);
      GLAPI GLvoid            glWindowPos3iv(GLint const *);
      GLAPI GLvoid            glWindowPos3s(GLshort, GLshort, GLshort);
      GLAPI GLvoid            glWindowPos3sv(GLshort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_VERSION_1_4 */


/* ---[ GL_VERSION_1_5 ]------------------------------------------------------------------------------------- */

/*! link : http://www.opengl.org/documentation/specs/version1.5/glspec15.pdf */

#ifndef GL_VERSION_1_5

   #define GL_VERSION_1_5 1
   #define GL_VERSION_1_5_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FOG_COORD_SOURCE

      #define GL_FOG_COORD_SOURCE                                         0x8450
      #define GL_FOG_COORD                                                0x8451
      #define GL_CURRENT_FOG_COORD                                        0x8453
      #define GL_FOG_COORD_ARRAY_TYPE                                     0x8454
      #define GL_FOG_COORD_ARRAY_STRIDE                                   0x8455
      #define GL_FOG_COORD_ARRAY_POINTER                                  0x8456
      #define GL_FOG_COORD_ARRAY                                          0x8457
      #define GL_SRC0_RGB                                                 0x8580
      #define GL_SRC1_RGB                                                 0x8581
      #define GL_SRC2_RGB                                                 0x8582
      #define GL_SRC0_ALPHA                                               0x8588
      #define GL_SRC1_ALPHA                                               0x8589
      #define GL_SRC2_ALPHA                                               0x858a
      #define GL_BUFFER_SIZE                                              0x8764
      #define GL_BUFFER_USAGE                                             0x8765
      #define GL_QUERY_COUNTER_BITS                                       0x8864
      #define GL_CURRENT_QUERY                                            0x8865
      #define GL_QUERY_RESULT                                             0x8866
      #define GL_QUERY_RESULT_AVAILABLE                                   0x8867
      #define GL_ARRAY_BUFFER                                             0x8892
      #define GL_ELEMENT_ARRAY_BUFFER                                     0x8893
      #define GL_ARRAY_BUFFER_BINDING                                     0x8894
      #define GL_ELEMENT_ARRAY_BUFFER_BINDING                             0x8895
      #define GL_VERTEX_ARRAY_BUFFER_BINDING                              0x8896
      #define GL_NORMAL_ARRAY_BUFFER_BINDING                              0x8897
      #define GL_COLOR_ARRAY_BUFFER_BINDING                               0x8898
      #define GL_INDEX_ARRAY_BUFFER_BINDING                               0x8899
      #define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING                       0x889a
      #define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING                           0x889b
      #define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING                     0x889c
      #define GL_FOG_COORD_ARRAY_BUFFER_BINDING                           0x889d
      #define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING                      0x889d
      #define GL_WEIGHT_ARRAY_BUFFER_BINDING                              0x889e
      #define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING                       0x889f
      #define GL_READ_ONLY                                                0x88b8
      #define GL_WRITE_ONLY                                               0x88b9
      #define GL_READ_WRITE                                               0x88ba
      #define GL_BUFFER_ACCESS                                            0x88bb
      #define GL_BUFFER_MAPPED                                            0x88bc
      #define GL_BUFFER_MAP_POINTER                                       0x88bd
      #define GL_STREAM_DRAW                                              0x88e0
      #define GL_STREAM_READ                                              0x88e1
      #define GL_STREAM_COPY                                              0x88e2
      #define GL_STATIC_DRAW                                              0x88e4
      #define GL_STATIC_READ                                              0x88e5
      #define GL_STATIC_COPY                                              0x88e6
      #define GL_DYNAMIC_DRAW                                             0x88e8
      #define GL_DYNAMIC_READ                                             0x88e9
      #define GL_DYNAMIC_COPY                                             0x88ea
      #define GL_SAMPLES_PASSED                                           0x8914

   #endif /* GL_FOG_COORD_SOURCE */

   /* - -[ type definitions ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   typedef ptrdiff_t                                                      GLintptr;
   typedef ptrdiff_t                                                      GLsizeiptr;

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBeginQuery                                     OGLEXT_MAKEGLNAME(BeginQuery)
      #define glBindBuffer                                     OGLEXT_MAKEGLNAME(BindBuffer)
      #define glBufferData                                     OGLEXT_MAKEGLNAME(BufferData)
      #define glBufferSubData                                  OGLEXT_MAKEGLNAME(BufferSubData)
      #define glDeleteBuffers                                  OGLEXT_MAKEGLNAME(DeleteBuffers)
      #define glDeleteQueries                                  OGLEXT_MAKEGLNAME(DeleteQueries)
      #define glEndQuery                                       OGLEXT_MAKEGLNAME(EndQuery)
      #define glGenBuffers                                     OGLEXT_MAKEGLNAME(GenBuffers)
      #define glGenQueries                                     OGLEXT_MAKEGLNAME(GenQueries)
      #define glGetBufferParameteriv                           OGLEXT_MAKEGLNAME(GetBufferParameteriv)
      #define glGetBufferPointerv                              OGLEXT_MAKEGLNAME(GetBufferPointerv)
      #define glGetBufferSubData                               OGLEXT_MAKEGLNAME(GetBufferSubData)
      #define glGetQueryiv                                     OGLEXT_MAKEGLNAME(GetQueryiv)
      #define glGetQueryObjectiv                               OGLEXT_MAKEGLNAME(GetQueryObjectiv)
      #define glGetQueryObjectuiv                              OGLEXT_MAKEGLNAME(GetQueryObjectuiv)
      #define glIsBuffer                                       OGLEXT_MAKEGLNAME(IsBuffer)
      #define glIsQuery                                        OGLEXT_MAKEGLNAME(IsQuery)
      #define glMapBuffer                                      OGLEXT_MAKEGLNAME(MapBuffer)
      #define glUnmapBuffer                                    OGLEXT_MAKEGLNAME(UnmapBuffer)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBeginQuery(GLenum, GLuint);
      GLAPI GLvoid            glBindBuffer(GLenum, GLuint);
      GLAPI GLvoid            glBufferData(GLenum, GLsizeiptr, GLvoid const *, GLenum);
      GLAPI GLvoid            glBufferSubData(GLenum, GLintptr, GLsizeiptr, GLvoid const *);
      GLAPI GLvoid            glDeleteBuffers(GLsizei, GLuint const *);
      GLAPI GLvoid            glDeleteQueries(GLsizei, GLuint const *);
      GLAPI GLvoid            glEndQuery(GLenum);
      GLAPI GLvoid            glGenBuffers(GLsizei, GLuint *);
      GLAPI GLvoid            glGenQueries(GLsizei, GLuint *);
      GLAPI GLvoid            glGetBufferParameteriv(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetBufferPointerv(GLenum, GLenum, GLvoid * *);
      GLAPI GLvoid            glGetBufferSubData(GLenum, GLintptr, GLsizeiptr, GLvoid *);
      GLAPI GLvoid            glGetQueryiv(GLenum, GLenum, GLint);
      GLAPI GLvoid            glGetQueryObjectiv(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetQueryObjectuiv(GLuint, GLenum, GLuint *);
      GLAPI GLboolean         glIsBuffer(GLuint);
      GLAPI GLboolean         glIsQuery(GLuint);
      GLAPI GLvoid *          glMapBuffer(GLenum, GLenum);
      GLAPI GLboolean         glUnmapBuffer(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_VERSION_1_5 */



/* ========================================================================================================== */
/* ===                                  O P E N G L   E X T E N S I O N                                   === */
/* ========================================================================================================== */

/* ---[ GL_3DFX_multisample ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/3DFX/3dfx_multisample.txt */

#ifndef GL_3DFX_multisample

   #define GL_3DFX_multisample 1
   #define GL_3DFX_multisample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MULTISAMPLE_3DFX

      #define GL_MULTISAMPLE_3DFX                                         0x86b2
      #define GL_SAMPLE_BUFFERS_3DFX                                      0x86b3
      #define GL_SAMPLES_3DFX                                             0x86b4
      #define GL_MULTISAMPLE_BIT_3DFX                                     0x20000000

   #endif /* GL_MULTISAMPLE_3DFX */

#endif /* GL_3DFX_multisample */


/* ---[ GL_3DFX_tbuffer ]------------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/3DFX/tbuffer.txt */

#ifndef GL_3DFX_tbuffer

   #define GL_3DFX_tbuffer 1
   #define GL_3DFX_tbuffer_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glTbufferMask3DFX                                OGLEXT_MAKEGLNAME(TbufferMask3DFX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glTbufferMask3DFX(GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_3DFX_tbuffer */


/* ---[ GL_3DFX_texture_compression_FXT1 ]------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/3DFX/texture_compression_FXT1.txt */

#ifndef GL_3DFX_texture_compression_FXT1

   #define GL_3DFX_texture_compression_FXT1 1
   #define GL_3DFX_texture_compression_FXT1_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COMPRESSED_RGB_FXT1_3DFX

      #define GL_COMPRESSED_RGB_FXT1_3DFX                                 0x86b0
      #define GL_COMPRESSED_RGBA_FXT1_3DFX                                0x86b1

   #endif /* GL_COMPRESSED_RGB_FXT1_3DFX */

#endif /* GL_3DFX_texture_compression_FXT1 */


/* ---[ GL_APPLE_client_storage ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/client_storage.txt */

#ifndef GL_APPLE_client_storage

   #define GL_APPLE_client_storage 1
   #define GL_APPLE_client_storage_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_UNPACK_CLIENT_STORAGE_APPLE

      #define GL_UNPACK_CLIENT_STORAGE_APPLE                              0x85b2

   #endif /* GL_UNPACK_CLIENT_STORAGE_APPLE */

#endif /* GL_APPLE_client_storage */


/* ---[ GL_APPLE_element_array ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/element_array.txt */

#ifndef GL_APPLE_element_array

   #define GL_APPLE_element_array 1
   #define GL_APPLE_element_array_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ELEMENT_ARRAY_APPLE

      #define GL_ELEMENT_ARRAY_APPLE                                      0x8768
      #define GL_ELEMENT_ARRAY_TYPE_APPLE                                 0x8769
      #define GL_ELEMENT_ARRAY_POINTER_APPLE                              0x876a

   #endif /* GL_ELEMENT_ARRAY_APPLE */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDrawElementArrayAPPLE                          OGLEXT_MAKEGLNAME(DrawElementArrayAPPLE)
      #define glDrawRangeElementArrayAPPLE                     OGLEXT_MAKEGLNAME(DrawRangeElementArrayAPPLE)
      #define glElementPointerAPPLE                            OGLEXT_MAKEGLNAME(ElementPointerAPPLE)
      #define glMultiDrawElementArrayAPPLE                     OGLEXT_MAKEGLNAME(MultiDrawElementArrayAPPLE)
      #define glMultiDrawRangeElementArrayAPPLE                OGLEXT_MAKEGLNAME(MultiDrawRangeElementArrayAPPLE)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDrawElementArrayAPPLE(GLenum, GLint, GLsizei);
      GLAPI GLvoid            glDrawRangeElementArrayAPPLE(GLenum, GLuint, GLuint, GLint, GLsizei);
      GLAPI GLvoid            glElementPointerAPPLE(GLenum, GLvoid const *);
      GLAPI GLvoid            glMultiDrawElementArrayAPPLE(GLenum, GLint const *, GLsizei const *, GLsizei);
      GLAPI GLvoid            glMultiDrawRangeElementArrayAPPLE(GLenum, GLuint, GLuint, GLint const *, GLsizei const *, GLsizei);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_APPLE_element_array */


/* ---[ GL_APPLE_fence ]------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/fence.txt */

#ifndef GL_APPLE_fence

   #define GL_APPLE_fence 1
   #define GL_APPLE_fence_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DRAW_PIXELS_APPLE

      #define GL_DRAW_PIXELS_APPLE                                        0x8a0a
      #define GL_FENCE_APPLE                                              0x8a0b

   #endif /* GL_DRAW_PIXELS_APPLE */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDeleteFencesAPPLE                              OGLEXT_MAKEGLNAME(DeleteFencesAPPLE)
      #define glFinishFenceAPPLE                               OGLEXT_MAKEGLNAME(FinishFenceAPPLE)
      #define glFinishObjectAPPLE                              OGLEXT_MAKEGLNAME(FinishObjectAPPLE)
      #define glGenFencesAPPLE                                 OGLEXT_MAKEGLNAME(GenFencesAPPLE)
      #define glIsFenceAPPLE                                   OGLEXT_MAKEGLNAME(IsFenceAPPLE)
      #define glSetFenceAPPLE                                  OGLEXT_MAKEGLNAME(SetFenceAPPLE)
      #define glTestFenceAPPLE                                 OGLEXT_MAKEGLNAME(TestFenceAPPLE)
      #define glTestObjectAPPLE                                OGLEXT_MAKEGLNAME(TestObjectAPPLE)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDeleteFencesAPPLE(GLsizei, GLuint const *);
      GLAPI GLvoid            glFinishFenceAPPLE(GLuint);
      GLAPI GLvoid            glFinishObjectAPPLE(GLenum, GLint);
      GLAPI GLvoid            glGenFencesAPPLE(GLsizei, GLuint *);
      GLAPI GLboolean         glIsFenceAPPLE(GLuint);
      GLAPI GLvoid            glSetFenceAPPLE(GLuint);
      GLAPI GLboolean         glTestFenceAPPLE(GLuint);
      GLAPI GLboolean         glTestObjectAPPLE(GLenum, GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_APPLE_fence */


/* ---[ GL_APPLE_specular_vector ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/specular_vector.txt */

#ifndef GL_APPLE_specular_vector

   #define GL_APPLE_specular_vector 1
   #define GL_APPLE_specular_vector_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE

      #define GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE                        0x85b0

   #endif /* GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE */

#endif /* GL_APPLE_specular_vector */


/* ---[ GL_APPLE_transform_hint ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/transform_hint.txt */

#ifndef GL_APPLE_transform_hint

   #define GL_APPLE_transform_hint 1
   #define GL_APPLE_transform_hint_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TRANSFORM_HINT_APPLE

      #define GL_TRANSFORM_HINT_APPLE                                     0x85b1

   #endif /* GL_TRANSFORM_HINT_APPLE */

#endif /* GL_APPLE_transform_hint */


/* ---[ GL_APPLE_vertex_array_object ]----------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/vertex_array_object.txt */

#ifndef GL_APPLE_vertex_array_object

   #define GL_APPLE_vertex_array_object 1
   #define GL_APPLE_vertex_array_object_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_ARRAY_BINDING_APPLE

      #define GL_VERTEX_ARRAY_BINDING_APPLE                               0x85b5

   #endif /* GL_VERTEX_ARRAY_BINDING_APPLE */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBindVertexArrayAPPLE                           OGLEXT_MAKEGLNAME(BindVertexArrayAPPLE)
      #define glDeleteVertexArraysAPPLE                        OGLEXT_MAKEGLNAME(DeleteVertexArraysAPPLE)
      #define glGenVertexArraysAPPLE                           OGLEXT_MAKEGLNAME(GenVertexArraysAPPLE)
      #define glIsVertexArrayAPPLE                             OGLEXT_MAKEGLNAME(IsVertexArrayAPPLE)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBindVertexArrayAPPLE(GLuint);
      GLAPI GLvoid            glDeleteVertexArraysAPPLE(GLsizei, GLuint const *);
      GLAPI GLvoid            glGenVertexArraysAPPLE(GLsizei, GLuint const *);
      GLAPI GLboolean         glIsVertexArrayAPPLE(GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_APPLE_vertex_array_object */


/* ---[ GL_APPLE_vertex_array_range ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/vertex_array_range.txt */

#ifndef GL_APPLE_vertex_array_range

   #define GL_APPLE_vertex_array_range 1
   #define GL_APPLE_vertex_array_range_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_ARRAY_RANGE_APPLE

      #define GL_VERTEX_ARRAY_RANGE_APPLE                                 0x851d
      #define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE                          0x851e
      #define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE                          0x851f
      #define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE                         0x8521
      #define GL_STORAGE_CACHED_APPLE                                     0x85be
      #define GL_STORAGE_SHARED_APPLE                                     0x85bf

   #endif /* GL_VERTEX_ARRAY_RANGE_APPLE */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFlushVertexArrayRangeAPPLE                     OGLEXT_MAKEGLNAME(FlushVertexArrayRangeAPPLE)
      #define glVertexArrayParameteriAPPLE                     OGLEXT_MAKEGLNAME(VertexArrayParameteriAPPLE)
      #define glVertexArrayRangeAPPLE                          OGLEXT_MAKEGLNAME(VertexArrayRangeAPPLE)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFlushVertexArrayRangeAPPLE(GLsizei, GLvoid *);
      GLAPI GLvoid            glVertexArrayParameteriAPPLE(GLenum, GLint);
      GLAPI GLvoid            glVertexArrayRangeAPPLE(GLsizei, GLvoid *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_APPLE_vertex_array_range */


/* ---[ GL_APPLE_ycbcr_422 ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/APPLE/ycbcr_422.txt */

#ifndef GL_APPLE_ycbcr_422

   #define GL_APPLE_ycbcr_422 1
   #define GL_APPLE_ycbcr_422_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_YCBCR_422_APPLE

      #define GL_YCBCR_422_APPLE                                          0x85b9
      #define GL_UNSIGNED_SHORT_8_8_APPLE                                 0x85ba
      #define GL_UNSIGNED_SHORT_8_8_REV_APPLE                             0x85bb

   #endif /* GL_YCBCR_422_APPLE */

#endif /* GL_APPLE_ycbcr_422 */


/* ---[ GL_ARB_depth_texture ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/depth_texture.txt */

#ifndef GL_ARB_depth_texture

   #define GL_ARB_depth_texture 1
   #define GL_ARB_depth_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DEPTH_COMPONENT16_ARB

      #define GL_DEPTH_COMPONENT16_ARB                                    0x81a5
      #define GL_DEPTH_COMPONENT24_ARB                                    0x81a6
      #define GL_DEPTH_COMPONENT32_ARB                                    0x81a7
      #define GL_TEXTURE_DEPTH_SIZE_ARB                                   0x884a
      #define GL_DEPTH_TEXTURE_MODE_ARB                                   0x884b

   #endif /* GL_DEPTH_COMPONENT16_ARB */

#endif /* GL_ARB_depth_texture */


/* ---[ GL_ARB_fragment_program ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/fragment_program.txt */

#ifndef GL_ARB_fragment_program

   #define GL_ARB_fragment_program 1
   #define GL_ARB_fragment_program_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FRAGMENT_PROGRAM_ARB

      #define GL_FRAGMENT_PROGRAM_ARB                                     0x8804
      #define GL_PROGRAM_ALU_INSTRUCTIONS_ARB                             0x8805
      #define GL_PROGRAM_TEX_INSTRUCTIONS_ARB                             0x8806
      #define GL_PROGRAM_TEX_INDIRECTIONS_ARB                             0x8807
      #define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB                      0x8808
      #define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB                      0x8809
      #define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB                      0x880a
      #define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB                         0x880b
      #define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB                         0x880c
      #define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB                         0x880d
      #define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB                  0x880e
      #define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB                  0x880f
      #define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB                  0x8810
      #define GL_MAX_TEXTURE_COORDS_ARB                                   0x8871
      #define GL_MAX_TEXTURE_IMAGE_UNITS_ARB                              0x8872

   #endif /* GL_FRAGMENT_PROGRAM_ARB */

#endif /* GL_ARB_fragment_program */


/* ---[ GL_ARB_matrix_palette ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/matrix_palette.txt */

#ifndef GL_ARB_matrix_palette

   #define GL_ARB_matrix_palette 1
   #define GL_ARB_matrix_palette_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MATRIX_PALETTE_ARB

      #define GL_MATRIX_PALETTE_ARB                                       0x8840
      #define GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB                       0x8841
      #define GL_MAX_PALETTE_MATRICES_ARB                                 0x8842
      #define GL_CURRENT_PALETTE_MATRIX_ARB                               0x8843
      #define GL_MATRIX_INDEX_ARRAY_ARB                                   0x8844
      #define GL_CURRENT_MATRIX_INDEX_ARB                                 0x8845
      #define GL_MATRIX_INDEX_ARRAY_SIZE_ARB                              0x8846
      #define GL_MATRIX_INDEX_ARRAY_TYPE_ARB                              0x8847
      #define GL_MATRIX_INDEX_ARRAY_STRIDE_ARB                            0x8848
      #define GL_MATRIX_INDEX_ARRAY_POINTER_ARB                           0x8849

   #endif /* GL_MATRIX_PALETTE_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glCurrentPaletteMatrixARB                        OGLEXT_MAKEGLNAME(CurrentPaletteMatrixARB)
      #define glMatrixIndexPointerARB                          OGLEXT_MAKEGLNAME(MatrixIndexPointerARB)
      #define glMatrixIndexubvARB                              OGLEXT_MAKEGLNAME(MatrixIndexubvARB)
      #define glMatrixIndexuivARB                              OGLEXT_MAKEGLNAME(MatrixIndexuivARB)
      #define glMatrixIndexusvARB                              OGLEXT_MAKEGLNAME(MatrixIndexusvARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glCurrentPaletteMatrixARB(GLint);
      GLAPI GLvoid            glMatrixIndexPointerARB(GLint, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glMatrixIndexubvARB(GLint, GLubyte const *);
      GLAPI GLvoid            glMatrixIndexuivARB(GLint, GLuint const *);
      GLAPI GLvoid            glMatrixIndexusvARB(GLint, GLushort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_matrix_palette */


/* ---[ GL_ARB_multisample ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/multisample.txt */

#ifndef GL_ARB_multisample

   #define GL_ARB_multisample 1
   #define GL_ARB_multisample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MULTISAMPLE_ARB

      #define GL_MULTISAMPLE_ARB                                          0x809d
      #define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB                             0x809e
      #define GL_SAMPLE_ALPHA_TO_ONE_ARB                                  0x809f
      #define GL_SAMPLE_COVERAGE_ARB                                      0x80a0
      #define GL_SAMPLE_BUFFERS_ARB                                       0x80a8
      #define GL_SAMPLES_ARB                                              0x80a9
      #define GL_SAMPLE_COVERAGE_VALUE_ARB                                0x80aa
      #define GL_SAMPLE_COVERAGE_INVERT_ARB                               0x80ab
      #define GL_MULTISAMPLE_BIT_ARB                                      0x20000000

   #endif /* GL_MULTISAMPLE_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glSampleCoverageARB                              OGLEXT_MAKEGLNAME(SampleCoverageARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glSampleCoverageARB(GLclampf, GLboolean);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_multisample */


/* ---[ GL_ARB_multitexture ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/multitexture.txt */

#ifndef GL_ARB_multitexture

   #define GL_ARB_multitexture 1
   #define GL_ARB_multitexture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE0_ARB

      #define GL_TEXTURE0_ARB                                             0x84c0
      #define GL_TEXTURE1_ARB                                             0x84c1
      #define GL_TEXTURE2_ARB                                             0x84c2
      #define GL_TEXTURE3_ARB                                             0x84c3
      #define GL_TEXTURE4_ARB                                             0x84c4
      #define GL_TEXTURE5_ARB                                             0x84c5
      #define GL_TEXTURE6_ARB                                             0x84c6
      #define GL_TEXTURE7_ARB                                             0x84c7
      #define GL_TEXTURE8_ARB                                             0x84c8
      #define GL_TEXTURE9_ARB                                             0x84c9
      #define GL_TEXTURE10_ARB                                            0x84ca
      #define GL_TEXTURE11_ARB                                            0x84cb
      #define GL_TEXTURE12_ARB                                            0x84cc
      #define GL_TEXTURE13_ARB                                            0x84cd
      #define GL_TEXTURE14_ARB                                            0x84ce
      #define GL_TEXTURE15_ARB                                            0x84cf
      #define GL_TEXTURE16_ARB                                            0x84d0
      #define GL_TEXTURE17_ARB                                            0x84d1
      #define GL_TEXTURE18_ARB                                            0x84d2
      #define GL_TEXTURE19_ARB                                            0x84d3
      #define GL_TEXTURE20_ARB                                            0x84d4
      #define GL_TEXTURE21_ARB                                            0x84d5
      #define GL_TEXTURE22_ARB                                            0x84d6
      #define GL_TEXTURE23_ARB                                            0x84d7
      #define GL_TEXTURE24_ARB                                            0x84d8
      #define GL_TEXTURE25_ARB                                            0x84d9
      #define GL_TEXTURE26_ARB                                            0x84da
      #define GL_TEXTURE27_ARB                                            0x84db
      #define GL_TEXTURE28_ARB                                            0x84dc
      #define GL_TEXTURE29_ARB                                            0x84dd
      #define GL_TEXTURE30_ARB                                            0x84de
      #define GL_TEXTURE31_ARB                                            0x84df
      #define GL_ACTIVE_TEXTURE_ARB                                       0x84e0
      #define GL_CLIENT_ACTIVE_TEXTURE_ARB                                0x84e1
      #define GL_MAX_TEXTURE_UNITS_ARB                                    0x84e2

   #endif /* GL_TEXTURE0_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glActiveTextureARB                               OGLEXT_MAKEGLNAME(ActiveTextureARB)
      #define glClientActiveTextureARB                         OGLEXT_MAKEGLNAME(ClientActiveTextureARB)
      #define glMultiTexCoord1dARB                             OGLEXT_MAKEGLNAME(MultiTexCoord1dARB)
      #define glMultiTexCoord1dvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord1dvARB)
      #define glMultiTexCoord1fARB                             OGLEXT_MAKEGLNAME(MultiTexCoord1fARB)
      #define glMultiTexCoord1fvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord1fvARB)
      #define glMultiTexCoord1iARB                             OGLEXT_MAKEGLNAME(MultiTexCoord1iARB)
      #define glMultiTexCoord1ivARB                            OGLEXT_MAKEGLNAME(MultiTexCoord1ivARB)
      #define glMultiTexCoord1sARB                             OGLEXT_MAKEGLNAME(MultiTexCoord1sARB)
      #define glMultiTexCoord1svARB                            OGLEXT_MAKEGLNAME(MultiTexCoord1svARB)
      #define glMultiTexCoord2dARB                             OGLEXT_MAKEGLNAME(MultiTexCoord2dARB)
      #define glMultiTexCoord2dvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord2dvARB)
      #define glMultiTexCoord2fARB                             OGLEXT_MAKEGLNAME(MultiTexCoord2fARB)
      #define glMultiTexCoord2fvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord2fvARB)
      #define glMultiTexCoord2iARB                             OGLEXT_MAKEGLNAME(MultiTexCoord2iARB)
      #define glMultiTexCoord2ivARB                            OGLEXT_MAKEGLNAME(MultiTexCoord2ivARB)
      #define glMultiTexCoord2sARB                             OGLEXT_MAKEGLNAME(MultiTexCoord2sARB)
      #define glMultiTexCoord2svARB                            OGLEXT_MAKEGLNAME(MultiTexCoord2svARB)
      #define glMultiTexCoord3dARB                             OGLEXT_MAKEGLNAME(MultiTexCoord3dARB)
      #define glMultiTexCoord3dvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord3dvARB)
      #define glMultiTexCoord3fARB                             OGLEXT_MAKEGLNAME(MultiTexCoord3fARB)
      #define glMultiTexCoord3fvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord3fvARB)
      #define glMultiTexCoord3iARB                             OGLEXT_MAKEGLNAME(MultiTexCoord3iARB)
      #define glMultiTexCoord3ivARB                            OGLEXT_MAKEGLNAME(MultiTexCoord3ivARB)
      #define glMultiTexCoord3sARB                             OGLEXT_MAKEGLNAME(MultiTexCoord3sARB)
      #define glMultiTexCoord3svARB                            OGLEXT_MAKEGLNAME(MultiTexCoord3svARB)
      #define glMultiTexCoord4dARB                             OGLEXT_MAKEGLNAME(MultiTexCoord4dARB)
      #define glMultiTexCoord4dvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord4dvARB)
      #define glMultiTexCoord4fARB                             OGLEXT_MAKEGLNAME(MultiTexCoord4fARB)
      #define glMultiTexCoord4fvARB                            OGLEXT_MAKEGLNAME(MultiTexCoord4fvARB)
      #define glMultiTexCoord4iARB                             OGLEXT_MAKEGLNAME(MultiTexCoord4iARB)
      #define glMultiTexCoord4ivARB                            OGLEXT_MAKEGLNAME(MultiTexCoord4ivARB)
      #define glMultiTexCoord4sARB                             OGLEXT_MAKEGLNAME(MultiTexCoord4sARB)
      #define glMultiTexCoord4svARB                            OGLEXT_MAKEGLNAME(MultiTexCoord4svARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glActiveTextureARB(GLenum);
      GLAPI GLvoid            glClientActiveTextureARB(GLenum);
      GLAPI GLvoid            glMultiTexCoord1dARB(GLenum, GLdouble);
      GLAPI GLvoid            glMultiTexCoord1dvARB(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord1fARB(GLenum, GLfloat);
      GLAPI GLvoid            glMultiTexCoord1fvARB(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord1iARB(GLenum, GLint);
      GLAPI GLvoid            glMultiTexCoord1ivARB(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord1sARB(GLenum, GLshort);
      GLAPI GLvoid            glMultiTexCoord1svARB(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord2dARB(GLenum, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord2dvARB(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord2fARB(GLenum, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord2fvARB(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord2iARB(GLenum, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord2ivARB(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord2sARB(GLenum, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord2svARB(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord3dARB(GLenum, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord3dvARB(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord3fARB(GLenum, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord3fvARB(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord3iARB(GLenum, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord3ivARB(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord3sARB(GLenum, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord3svARB(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord4dARB(GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord4dvARB(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord4fARB(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord4fvARB(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord4iARB(GLenum, GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord4ivARB(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord4sARB(GLenum, GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord4svARB(GLenum, GLshort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_multitexture */


/* ---[ GL_ARB_occlusion_query ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/occlusion_query.txt */

#ifndef GL_ARB_occlusion_query

   #define GL_ARB_occlusion_query 1
   #define GL_ARB_occlusion_query_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBeginQueryARB                                  OGLEXT_MAKEGLNAME(BeginQueryARB)
      #define glDeleteQueriesARB                               OGLEXT_MAKEGLNAME(DeleteQueriesARB)
      #define glEndQueryARB                                    OGLEXT_MAKEGLNAME(EndQueryARB)
      #define glGenQueriesARB                                  OGLEXT_MAKEGLNAME(GenQueriesARB)
      #define glGetQueryivARB                                  OGLEXT_MAKEGLNAME(GetQueryivARB)
      #define glGetQueryObjectivARB                            OGLEXT_MAKEGLNAME(GetQueryObjectivARB)
      #define glGetQueryObjectuivARB                           OGLEXT_MAKEGLNAME(GetQueryObjectuivARB)
      #define glIsQueryARB                                     OGLEXT_MAKEGLNAME(IsQueryARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBeginQueryARB(GLenum, GLuint);
      GLAPI GLvoid            glDeleteQueriesARB(GLsizei, GLuint const *);
      GLAPI GLvoid            glEndQueryARB(GLenum);
      GLAPI GLvoid            glGenQueriesARB(GLsizei, GLuint *);
      GLAPI GLvoid            glGetQueryivARB(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetQueryObjectivARB(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetQueryObjectuivARB(GLuint, GLenum, GLuint *);
      GLAPI GLboolean         glIsQueryARB(GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_occlusion_query */


/* ---[ GL_ARB_point_parameters ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/point_parameters.txt */

#ifndef GL_ARB_point_parameters

   #define GL_ARB_point_parameters 1
   #define GL_ARB_point_parameters_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_POINT_SIZE_MIN_ARB

      #define GL_POINT_SIZE_MIN_ARB                                       0x8126
      #define GL_POINT_SIZE_MAX_ARB                                       0x8127
      #define GL_POINT_FADE_THRESHOLD_SIZE_ARB                            0x8128
      #define GL_POINT_DISTANCE_ATTENUATION_ARB                           0x8129

   #endif /* GL_POINT_SIZE_MIN_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPointParameterfARB                             OGLEXT_MAKEGLNAME(PointParameterfARB)
      #define glPointParameterfvARB                            OGLEXT_MAKEGLNAME(PointParameterfvARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPointParameterfARB(GLenum, GLfloat);
      GLAPI GLvoid            glPointParameterfvARB(GLenum, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_point_parameters */


/* ---[ GL_ARB_point_sprite ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/point_sprite.txt */

#ifndef GL_ARB_point_sprite

   #define GL_ARB_point_sprite 1
   #define GL_ARB_point_sprite_OGLEXT 1

#endif /* GL_ARB_point_sprite */


/* ---[ GL_ARB_shader_objects ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/shader_objects.txt */

#ifndef GL_ARB_shader_objects

   #define GL_ARB_shader_objects 1
   #define GL_ARB_shader_objects_OGLEXT 1

   /* - -[ type definitions ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   typedef char                                                           GLcharARB;
   typedef void *                                                         GLhandleARB;

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glAttachObjectARB                                OGLEXT_MAKEGLNAME(AttachObjectARB)
      #define glCompileShaderARB                               OGLEXT_MAKEGLNAME(CompileShaderARB)
      #define glCreateProgramObjectARB                         OGLEXT_MAKEGLNAME(CreateProgramObjectARB)
      #define glCreateShaderObjectARB                          OGLEXT_MAKEGLNAME(CreateShaderObjectARB)
      #define glDeleteObjectARB                                OGLEXT_MAKEGLNAME(DeleteObjectARB)
      #define glDetachObjectARB                                OGLEXT_MAKEGLNAME(DetachObjectARB)
      #define glGetActiveUniformARB                            OGLEXT_MAKEGLNAME(GetActiveUniformARB)
      #define glGetAttachedObjectsARB                          OGLEXT_MAKEGLNAME(GetAttachedObjectsARB)
      #define glGetHandleARB                                   OGLEXT_MAKEGLNAME(GetHandleARB)
      #define glGetInfoLogARB                                  OGLEXT_MAKEGLNAME(GetInfoLogARB)
      #define glGetObjectParameterfvARB                        OGLEXT_MAKEGLNAME(GetObjectParameterfvARB)
      #define glGetObjectParameterivARB                        OGLEXT_MAKEGLNAME(GetObjectParameterivARB)
      #define glGetShaderSourceARB                             OGLEXT_MAKEGLNAME(GetShaderSourceARB)
      #define glGetUniformfvARB                                OGLEXT_MAKEGLNAME(GetUniformfvARB)
      #define glGetUniformivARB                                OGLEXT_MAKEGLNAME(GetUniformivARB)
      #define glGetUniformLocationARB                          OGLEXT_MAKEGLNAME(GetUniformLocationARB)
      #define glLinkProgramARB                                 OGLEXT_MAKEGLNAME(LinkProgramARB)
      #define glShaderSourceARB                                OGLEXT_MAKEGLNAME(ShaderSourceARB)
      #define glUniform1fARB                                   OGLEXT_MAKEGLNAME(Uniform1fARB)
      #define glUniform1fvARB                                  OGLEXT_MAKEGLNAME(Uniform1fvARB)
      #define glUniform1iARB                                   OGLEXT_MAKEGLNAME(Uniform1iARB)
      #define glUniform1ivARB                                  OGLEXT_MAKEGLNAME(Uniform1ivARB)
      #define glUniform2fARB                                   OGLEXT_MAKEGLNAME(Uniform2fARB)
      #define glUniform2fvARB                                  OGLEXT_MAKEGLNAME(Uniform2fvARB)
      #define glUniform2iARB                                   OGLEXT_MAKEGLNAME(Uniform2iARB)
      #define glUniform2ivARB                                  OGLEXT_MAKEGLNAME(Uniform2ivARB)
      #define glUniform3fARB                                   OGLEXT_MAKEGLNAME(Uniform3fARB)
      #define glUniform3fvARB                                  OGLEXT_MAKEGLNAME(Uniform3fvARB)
      #define glUniform3iARB                                   OGLEXT_MAKEGLNAME(Uniform3iARB)
      #define glUniform3ivARB                                  OGLEXT_MAKEGLNAME(Uniform3ivARB)
      #define glUniform4fARB                                   OGLEXT_MAKEGLNAME(Uniform4fARB)
      #define glUniform4fvARB                                  OGLEXT_MAKEGLNAME(Uniform4fvARB)
      #define glUniform4iARB                                   OGLEXT_MAKEGLNAME(Uniform4iARB)
      #define glUniform4ivARB                                  OGLEXT_MAKEGLNAME(Uniform4ivARB)
      #define glUniformMatrix2fvARB                            OGLEXT_MAKEGLNAME(UniformMatrix2fvARB)
      #define glUniformMatrix3fvARB                            OGLEXT_MAKEGLNAME(UniformMatrix3fvARB)
      #define glUniformMatrix4fvARB                            OGLEXT_MAKEGLNAME(UniformMatrix4fvARB)
      #define glUseProgramObjectARB                            OGLEXT_MAKEGLNAME(UseProgramObjectARB)
      #define glValidateProgramARB                             OGLEXT_MAKEGLNAME(ValidateProgramARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glAttachObjectARB(GLhandleARB, GLhandleARB);
      GLAPI GLvoid            glCompileShaderARB(GLhandleARB);
      GLAPI GLhandleARB       glCreateProgramObjectARB();
      GLAPI GLhandleARB       glCreateShaderObjectARB(GLenum);
      GLAPI GLvoid            glDeleteObjectARB(GLhandleARB);
      GLAPI GLvoid            glDetachObjectARB(GLhandleARB, GLhandleARB);
      GLAPI GLvoid            glGetActiveUniformARB(GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
      GLAPI GLvoid            glGetAttachedObjectsARB(GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
      GLAPI GLhandleARB       glGetHandleARB(GLenum);
      GLAPI GLvoid            glGetInfoLogARB(GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
      GLAPI GLvoid            glGetObjectParameterfvARB(GLhandleARB, GLenum, GLfloat *);
      GLAPI GLvoid            glGetObjectParameterivARB(GLhandleARB, GLenum, GLint *);
      GLAPI GLvoid            glGetShaderSourceARB(GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
      GLAPI GLvoid            glGetUniformfvARB(GLhandleARB, GLint, GLfloat *);
      GLAPI GLvoid            glGetUniformivARB(GLhandleARB, GLint, GLint *);
      GLAPI GLint             glGetUniformLocationARB(GLhandleARB, GLcharARB const);
      GLAPI GLvoid            glLinkProgramARB(GLhandleARB);
      GLAPI GLvoid            glShaderSourceARB(GLhandleARB, GLsizei, GLcharARB const, GLint const *);
      GLAPI GLvoid            glUniform1fARB(GLint, GLfloat);
      GLAPI GLvoid            glUniform1fvARB(GLint, GLsizei, GLfloat *);
      GLAPI GLvoid            glUniform1iARB(GLint, GLint);
      GLAPI GLvoid            glUniform1ivARB(GLint, GLsizei, GLint *);
      GLAPI GLvoid            glUniform2fARB(GLint, GLfloat, GLfloat);
      GLAPI GLvoid            glUniform2fvARB(GLint, GLsizei, GLfloat *);
      GLAPI GLvoid            glUniform2iARB(GLint, GLint, GLint);
      GLAPI GLvoid            glUniform2ivARB(GLint, GLsizei, GLint *);
      GLAPI GLvoid            glUniform3fARB(GLint, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glUniform3fvARB(GLint, GLsizei, GLfloat *);
      GLAPI GLvoid            glUniform3iARB(GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glUniform3ivARB(GLint, GLsizei, GLint *);
      GLAPI GLvoid            glUniform4fARB(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glUniform4fvARB(GLint, GLsizei, GLfloat *);
      GLAPI GLvoid            glUniform4iARB(GLint, GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glUniform4ivARB(GLint, GLsizei, GLint *);
      GLAPI GLvoid            glUniformMatrix2fvARB(GLint, GLsizei, GLboolean, GLfloat *);
      GLAPI GLvoid            glUniformMatrix3fvARB(GLint, GLsizei, GLboolean, GLfloat *);
      GLAPI GLvoid            glUniformMatrix4fvARB(GLint, GLsizei, GLboolean, GLfloat *);
      GLAPI GLvoid            glUseProgramObjectARB(GLhandleARB);
      GLAPI GLvoid            glValidateProgramARB(GLhandleARB);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_shader_objects */


/* ---[ GL_ARB_shading_language_100 ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/shading_language_100.txt */

#ifndef GL_ARB_shading_language_100

   #define GL_ARB_shading_language_100 1
   #define GL_ARB_shading_language_100_OGLEXT 1

#endif /* GL_ARB_shading_language_100 */


/* ---[ GL_ARB_shadow ]-------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/shadow.txt */

#ifndef GL_ARB_shadow

   #define GL_ARB_shadow 1
   #define GL_ARB_shadow_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_COMPARE_MODE_ARB

      #define GL_TEXTURE_COMPARE_MODE_ARB                                 0x884c
      #define GL_TEXTURE_COMPARE_FUNC_ARB                                 0x884d
      #define GL_COMPARE_R_TO_TEXTURE_ARB                                 0x884e

   #endif /* GL_TEXTURE_COMPARE_MODE_ARB */

#endif /* GL_ARB_shadow */


/* ---[ GL_ARB_shadow_ambient ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/shadow_ambient.txt */

#ifndef GL_ARB_shadow_ambient

   #define GL_ARB_shadow_ambient 1
   #define GL_ARB_shadow_ambient_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_COMPARE_FAIL_VALUE_ARB

      #define GL_TEXTURE_COMPARE_FAIL_VALUE_ARB                           0x80bf

   #endif /* GL_TEXTURE_COMPARE_FAIL_VALUE_ARB */

#endif /* GL_ARB_shadow_ambient */


/* ---[ GL_ARB_texture_border_clamp ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_border_clamp.txt */

#ifndef GL_ARB_texture_border_clamp

   #define GL_ARB_texture_border_clamp 1
   #define GL_ARB_texture_border_clamp_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CLAMP_TO_BORDER_ARB

      #define GL_CLAMP_TO_BORDER_ARB                                      0x812d

   #endif /* GL_CLAMP_TO_BORDER_ARB */

#endif /* GL_ARB_texture_border_clamp */


/* ---[ GL_ARB_texture_compression ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_compression.txt */

#ifndef GL_ARB_texture_compression

   #define GL_ARB_texture_compression 1
   #define GL_ARB_texture_compression_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COMPRESSED_ALPHA_ARB

      #define GL_COMPRESSED_ALPHA_ARB                                     0x84e9
      #define GL_COMPRESSED_LUMINANCE_ARB                                 0x84ea
      #define GL_COMPRESSED_LUMINANCE_ALPHA_ARB                           0x84eb
      #define GL_COMPRESSED_INTENSITY_ARB                                 0x84ec
      #define GL_COMPRESSED_RGB_ARB                                       0x84ed
      #define GL_COMPRESSED_RGBA_ARB                                      0x84ee
      #define GL_TEXTURE_COMPRESSION_HINT_ARB                             0x84ef
      #define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB                        0x86a0
      #define GL_TEXTURE_COMPRESSED_ARB                                   0x86a1
      #define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB                       0x86a2
      #define GL_COMPRESSED_TEXTURE_FORMATS_ARB                           0x86a3

   #endif /* GL_COMPRESSED_ALPHA_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glCompressedTexImage1DARB                        OGLEXT_MAKEGLNAME(CompressedTexImage1DARB)
      #define glCompressedTexImage2DARB                        OGLEXT_MAKEGLNAME(CompressedTexImage2DARB)
      #define glCompressedTexImage3DARB                        OGLEXT_MAKEGLNAME(CompressedTexImage3DARB)
      #define glCompressedTexSubImage1DARB                     OGLEXT_MAKEGLNAME(CompressedTexSubImage1DARB)
      #define glCompressedTexSubImage2DARB                     OGLEXT_MAKEGLNAME(CompressedTexSubImage2DARB)
      #define glCompressedTexSubImage3DARB                     OGLEXT_MAKEGLNAME(CompressedTexSubImage3DARB)
      #define glGetCompressedTexImageARB                       OGLEXT_MAKEGLNAME(GetCompressedTexImageARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glCompressedTexImage1DARB(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexImage2DARB(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexImage3DARB(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexSubImage1DARB(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexSubImage2DARB(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glCompressedTexSubImage3DARB(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glGetCompressedTexImageARB(GLenum, GLint, GLvoid *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_texture_compression */


/* ---[ GL_ARB_texture_cube_map ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_cube_map.txt */

#ifndef GL_ARB_texture_cube_map

   #define GL_ARB_texture_cube_map 1
   #define GL_ARB_texture_cube_map_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_NORMAL_MAP_ARB

      #define GL_NORMAL_MAP_ARB                                           0x8511
      #define GL_REFLECTION_MAP_ARB                                       0x8512
      #define GL_TEXTURE_CUBE_MAP_ARB                                     0x8513
      #define GL_TEXTURE_BINDING_CUBE_MAP_ARB                             0x8514
      #define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB                          0x8515
      #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB                          0x8516
      #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB                          0x8517
      #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB                          0x8518
      #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB                          0x8519
      #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB                          0x851a
      #define GL_PROXY_TEXTURE_CUBE_MAP_ARB                               0x851b
      #define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB                            0x851c

   #endif /* GL_NORMAL_MAP_ARB */

#endif /* GL_ARB_texture_cube_map */


/* ---[ GL_ARB_texture_env_add ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_add.txt */

#ifndef GL_ARB_texture_env_add

   #define GL_ARB_texture_env_add 1
   #define GL_ARB_texture_env_add_OGLEXT 1

#endif /* GL_ARB_texture_env_add */


/* ---[ GL_ARB_texture_env_combine ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_combine.txt */

#ifndef GL_ARB_texture_env_combine

   #define GL_ARB_texture_env_combine 1
   #define GL_ARB_texture_env_combine_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SUBTRACT_ARB

      #define GL_SUBTRACT_ARB                                             0x84e7
      #define GL_COMBINE_ARB                                              0x8570
      #define GL_COMBINE_RGB_ARB                                          0x8571
      #define GL_COMBINE_ALPHA_ARB                                        0x8572
      #define GL_RGB_SCALE_ARB                                            0x8573
      #define GL_ADD_SIGNED_ARB                                           0x8574
      #define GL_INTERPOLATE_ARB                                          0x8575
      #define GL_CONSTANT_ARB                                             0x8576
      #define GL_PRIMARY_COLOR_ARB                                        0x8577
      #define GL_PREVIOUS_ARB                                             0x8578
      #define GL_SOURCE0_RGB_ARB                                          0x8580
      #define GL_SOURCE1_RGB_ARB                                          0x8581
      #define GL_SOURCE2_RGB_ARB                                          0x8582
      #define GL_SOURCE0_ALPHA_ARB                                        0x8588
      #define GL_SOURCE1_ALPHA_ARB                                        0x8589
      #define GL_SOURCE2_ALPHA_ARB                                        0x858a
      #define GL_OPERAND0_RGB_ARB                                         0x8590
      #define GL_OPERAND1_RGB_ARB                                         0x8591
      #define GL_OPERAND2_RGB_ARB                                         0x8592
      #define GL_OPERAND0_ALPHA_ARB                                       0x8598
      #define GL_OPERAND1_ALPHA_ARB                                       0x8599
      #define GL_OPERAND2_ALPHA_ARB                                       0x859a

   #endif /* GL_SUBTRACT_ARB */

#endif /* GL_ARB_texture_env_combine */


/* ---[ GL_ARB_texture_env_crossbar ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_crossbar.txt */

#ifndef GL_ARB_texture_env_crossbar

   #define GL_ARB_texture_env_crossbar 1
   #define GL_ARB_texture_env_crossbar_OGLEXT 1

#endif /* GL_ARB_texture_env_crossbar */


/* ---[ GL_ARB_texture_env_dot3 ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_dot3.txt */

#ifndef GL_ARB_texture_env_dot3

   #define GL_ARB_texture_env_dot3 1
   #define GL_ARB_texture_env_dot3_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DOT3_RGB_ARB

      #define GL_DOT3_RGB_ARB                                             0x86ae
      #define GL_DOT3_RGBA_ARB                                            0x86af

   #endif /* GL_DOT3_RGB_ARB */

#endif /* GL_ARB_texture_env_dot3 */


/* ---[ GL_ARB_texture_mirror_repeat ]----------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_mirrored_repeat.txt */

#ifndef GL_ARB_texture_mirror_repeat

   #define GL_ARB_texture_mirror_repeat 1
   #define GL_ARB_texture_mirror_repeat_OGLEXT 1

#endif /* GL_ARB_texture_mirror_repeat */


/* ---[ GL_ARB_texture_non_power_of_two ]-------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_non_power_of_two.txt */

#ifndef GL_ARB_texture_non_power_of_two

   #define GL_ARB_texture_non_power_of_two 1
   #define GL_ARB_texture_non_power_of_two_OGLEXT 1

#endif /* GL_ARB_texture_non_power_of_two */


/* ---[ GL_ARB_transpose_matrix ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/transpose_matrix.txt */

#ifndef GL_ARB_transpose_matrix

   #define GL_ARB_transpose_matrix 1
   #define GL_ARB_transpose_matrix_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TRANSPOSE_MODELVIEW_MATRIX_ARB

      #define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB                           0x84e3
      #define GL_TRANSPOSE_PROJECTION_MATRIX_ARB                          0x84e4
      #define GL_TRANSPOSE_TEXTURE_MATRIX_ARB                             0x84e5
      #define GL_TRANSPOSE_COLOR_MATRIX_ARB                               0x84e6

   #endif /* GL_TRANSPOSE_MODELVIEW_MATRIX_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glLoadTransposeMatrixdARB                        OGLEXT_MAKEGLNAME(LoadTransposeMatrixdARB)
      #define glLoadTransposeMatrixfARB                        OGLEXT_MAKEGLNAME(LoadTransposeMatrixfARB)
      #define glMultTransposeMatrixdARB                        OGLEXT_MAKEGLNAME(MultTransposeMatrixdARB)
      #define glMultTransposeMatrixfARB                        OGLEXT_MAKEGLNAME(MultTransposeMatrixfARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glLoadTransposeMatrixdARB(GLdouble const *);
      GLAPI GLvoid            glLoadTransposeMatrixfARB(GLfloat const *);
      GLAPI GLvoid            glMultTransposeMatrixdARB(GLdouble const *);
      GLAPI GLvoid            glMultTransposeMatrixfARB(GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_transpose_matrix */


/* ---[ GL_ARB_vertex_blend ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/vertex_blend.txt */

#ifndef GL_ARB_vertex_blend

   #define GL_ARB_vertex_blend 1
   #define GL_ARB_vertex_blend_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MODELVIEW0_ARB

      #define GL_MODELVIEW0_ARB                                           0x1700
      #define GL_MODELVIEW1_ARB                                           0x850a
      #define GL_MAX_VERTEX_UNITS_ARB                                     0x86a4
      #define GL_ACTIVE_VERTEX_UNITS_ARB                                  0x86a5
      #define GL_WEIGHT_SUM_UNITY_ARB                                     0x86a6
      #define GL_VERTEX_BLEND_ARB                                         0x86a7
      #define GL_CURRENT_WEIGHT_ARB                                       0x86a8
      #define GL_WEIGHT_ARRAY_TYPE_ARB                                    0x86a9
      #define GL_WEIGHT_ARRAY_STRIDE_ARB                                  0x86aa
      #define GL_WEIGHT_ARRAY_SIZE_ARB                                    0x86ab
      #define GL_WEIGHT_ARRAY_POINTER_ARB                                 0x86ac
      #define GL_WEIGHT_ARRAY_ARB                                         0x86ad
      #define GL_MODELVIEW2_ARB                                           0x8722
      #define GL_MODELVIEW3_ARB                                           0x8723
      #define GL_MODELVIEW4_ARB                                           0x8724
      #define GL_MODELVIEW5_ARB                                           0x8725
      #define GL_MODELVIEW6_ARB                                           0x8726
      #define GL_MODELVIEW7_ARB                                           0x8727
      #define GL_MODELVIEW8_ARB                                           0x8728
      #define GL_MODELVIEW9_ARB                                           0x8729
      #define GL_MODELVIEW10_ARB                                          0x872a
      #define GL_MODELVIEW11_ARB                                          0x872b
      #define GL_MODELVIEW12_ARB                                          0x872c
      #define GL_MODELVIEW13_ARB                                          0x872d
      #define GL_MODELVIEW14_ARB                                          0x872e
      #define GL_MODELVIEW15_ARB                                          0x872f
      #define GL_MODELVIEW16_ARB                                          0x8730
      #define GL_MODELVIEW17_ARB                                          0x8731
      #define GL_MODELVIEW18_ARB                                          0x8732
      #define GL_MODELVIEW19_ARB                                          0x8733
      #define GL_MODELVIEW20_ARB                                          0x8734
      #define GL_MODELVIEW21_ARB                                          0x8735
      #define GL_MODELVIEW22_ARB                                          0x8736
      #define GL_MODELVIEW23_ARB                                          0x8737
      #define GL_MODELVIEW24_ARB                                          0x8738
      #define GL_MODELVIEW25_ARB                                          0x8739
      #define GL_MODELVIEW26_ARB                                          0x873a
      #define GL_MODELVIEW27_ARB                                          0x873b
      #define GL_MODELVIEW28_ARB                                          0x873c
      #define GL_MODELVIEW29_ARB                                          0x873d
      #define GL_MODELVIEW30_ARB                                          0x873e
      #define GL_MODELVIEW31_ARB                                          0x873f

   #endif /* GL_MODELVIEW0_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glVertexBlendARB                                 OGLEXT_MAKEGLNAME(VertexBlendARB)
      #define glWeightbvARB                                    OGLEXT_MAKEGLNAME(WeightbvARB)
      #define glWeightdvARB                                    OGLEXT_MAKEGLNAME(WeightdvARB)
      #define glWeightfvARB                                    OGLEXT_MAKEGLNAME(WeightfvARB)
      #define glWeightivARB                                    OGLEXT_MAKEGLNAME(WeightivARB)
      #define glWeightPointerARB                               OGLEXT_MAKEGLNAME(WeightPointerARB)
      #define glWeightsvARB                                    OGLEXT_MAKEGLNAME(WeightsvARB)
      #define glWeightubvARB                                   OGLEXT_MAKEGLNAME(WeightubvARB)
      #define glWeightuivARB                                   OGLEXT_MAKEGLNAME(WeightuivARB)
      #define glWeightusvARB                                   OGLEXT_MAKEGLNAME(WeightusvARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glVertexBlendARB(GLint);
      GLAPI GLvoid            glWeightbvARB(GLint, GLbyte const *);
      GLAPI GLvoid            glWeightdvARB(GLint, GLdouble const *);
      GLAPI GLvoid            glWeightfvARB(GLint, GLfloat const *);
      GLAPI GLvoid            glWeightivARB(GLint, GLint const *);
      GLAPI GLvoid            glWeightPointerARB(GLint, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glWeightsvARB(GLint, GLshort const *);
      GLAPI GLvoid            glWeightubvARB(GLint, GLubyte const *);
      GLAPI GLvoid            glWeightuivARB(GLint, GLuint const *);
      GLAPI GLvoid            glWeightusvARB(GLint, GLushort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_vertex_blend */


/* ---[ GL_ARB_vertex_buffer_object ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/vertex_buffer_object.txt */

#ifndef GL_ARB_vertex_buffer_object

   #define GL_ARB_vertex_buffer_object 1
   #define GL_ARB_vertex_buffer_object_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_BUFFER_SIZE_ARB

      #define GL_BUFFER_SIZE_ARB                                          0x8764
      #define GL_BUFFER_USAGE_ARB                                         0x8765
      #define GL_ARRAY_BUFFER_ARB                                         0x8892
      #define GL_ELEMENT_ARRAY_BUFFER_ARB                                 0x8893
      #define GL_ARRAY_BUFFER_BINDING_ARB                                 0x8894
      #define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB                         0x8895
      #define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB                          0x8896
      #define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB                          0x8897
      #define GL_COLOR_ARRAY_BUFFER_BINDING_ARB                           0x8898
      #define GL_INDEX_ARRAY_BUFFER_BINDING_ARB                           0x8899
      #define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB                   0x889a
      #define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB                       0x889b
      #define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB                 0x889c
      #define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB                  0x889d
      #define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB                          0x889e
      #define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB                   0x889f
      #define GL_READ_ONLY_ARB                                            0x88b8
      #define GL_WRITE_ONLY_ARB                                           0x88b9
      #define GL_READ_WRITE_ARB                                           0x88ba
      #define GL_BUFFER_ACCESS_ARB                                        0x88bb
      #define GL_BUFFER_MAPPED_ARB                                        0x88bc
      #define GL_BUFFER_MAP_POINTER_ARB                                   0x88bd
      #define GL_STREAM_DRAW_ARB                                          0x88e0
      #define GL_STREAM_READ_ARB                                          0x88e1
      #define GL_STREAM_COPY_ARB                                          0x88e2
      #define GL_STATIC_DRAW_ARB                                          0x88e4
      #define GL_STATIC_READ_ARB                                          0x88e5
      #define GL_STATIC_COPY_ARB                                          0x88e6
      #define GL_DYNAMIC_DRAW_ARB                                         0x88e8
      #define GL_DYNAMIC_READ_ARB                                         0x88e9
      #define GL_DYNAMIC_COPY_ARB                                         0x88ea

   #endif /* GL_BUFFER_SIZE_ARB */

   /* - -[ type definitions ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   typedef ptrdiff_t                                                      GLintptrARB;
   typedef ptrdiff_t                                                      GLsizeiptrARB;

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBindBufferARB                                  OGLEXT_MAKEGLNAME(BindBufferARB)
      #define glBufferDataARB                                  OGLEXT_MAKEGLNAME(BufferDataARB)
      #define glBufferSubDataARB                               OGLEXT_MAKEGLNAME(BufferSubDataARB)
      #define glDeleteBuffersARB                               OGLEXT_MAKEGLNAME(DeleteBuffersARB)
      #define glGenBuffersARB                                  OGLEXT_MAKEGLNAME(GenBuffersARB)
      #define glGetBufferParameterivARB                        OGLEXT_MAKEGLNAME(GetBufferParameterivARB)
      #define glGetBufferPointervARB                           OGLEXT_MAKEGLNAME(GetBufferPointervARB)
      #define glGetBufferSubDataARB                            OGLEXT_MAKEGLNAME(GetBufferSubDataARB)
      #define glIsBufferARB                                    OGLEXT_MAKEGLNAME(IsBufferARB)
      #define glMapBufferARB                                   OGLEXT_MAKEGLNAME(MapBufferARB)
      #define glUnmapBufferARB                                 OGLEXT_MAKEGLNAME(UnmapBufferARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBindBufferARB(GLenum, GLuint);
      GLAPI GLvoid            glBufferDataARB(GLenum, GLsizeiptrARB, GLvoid const *, GLenum);
      GLAPI GLvoid            glBufferSubDataARB(GLenum, GLintptrARB, GLsizeiptrARB, GLvoid const *);
      GLAPI GLvoid            glDeleteBuffersARB(GLsizei, GLuint const *);
      GLAPI GLvoid            glGenBuffersARB(GLsizei, GLuint *);
      GLAPI GLvoid            glGetBufferParameterivARB(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetBufferPointervARB(GLenum, GLenum, GLvoid * *);
      GLAPI GLvoid            glGetBufferSubDataARB(GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *);
      GLAPI GLboolean         glIsBufferARB(GLuint);
      GLAPI GLvoid *          glMapBufferARB(GLenum, GLenum);
      GLAPI GLboolean         glUnmapBufferARB(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_vertex_buffer_object */


/* ---[ GL_ARB_vertex_program ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/vertex_program.txt */

#ifndef GL_ARB_vertex_program

   #define GL_ARB_vertex_program 1
   #define GL_ARB_vertex_program_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COLOR_SUM_ARB

      #define GL_COLOR_SUM_ARB                                            0x8458
      #define GL_VERTEX_PROGRAM_ARB                                       0x8620
      #define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB                          0x8622
      #define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB                             0x8623
      #define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB                           0x8624
      #define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB                             0x8625
      #define GL_CURRENT_VERTEX_ATTRIB_ARB                                0x8626
      #define GL_PROGRAM_LENGTH_ARB                                       0x8627
      #define GL_PROGRAM_STRING_ARB                                       0x8628
      #define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB                       0x862e
      #define GL_MAX_PROGRAM_MATRICES_ARB                                 0x862f
      #define GL_CURRENT_MATRIX_STACK_DEPTH_ARB                           0x8640
      #define GL_CURRENT_MATRIX_ARB                                       0x8641
      #define GL_VERTEX_PROGRAM_POINT_SIZE_ARB                            0x8642
      #define GL_VERTEX_PROGRAM_TWO_SIDE_ARB                              0x8643
      #define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB                          0x8645
      #define GL_PROGRAM_ERROR_POSITION_ARB                               0x864b
      #define GL_PROGRAM_BINDING_ARB                                      0x8677
      #define GL_MAX_VERTEX_ATTRIBS_ARB                                   0x8869
      #define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB                       0x886a
      #define GL_PROGRAM_ERROR_STRING_ARB                                 0x8874
      #define GL_PROGRAM_FORMAT_ASCII_ARB                                 0x8875
      #define GL_PROGRAM_FORMAT_ARB                                       0x8876
      #define GL_PROGRAM_INSTRUCTIONS_ARB                                 0x88a0
      #define GL_MAX_PROGRAM_INSTRUCTIONS_ARB                             0x88a1
      #define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB                          0x88a2
      #define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB                      0x88a3
      #define GL_PROGRAM_TEMPORARIES_ARB                                  0x88a4
      #define GL_MAX_PROGRAM_TEMPORARIES_ARB                              0x88a5
      #define GL_PROGRAM_NATIVE_TEMPORARIES_ARB                           0x88a6
      #define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB                       0x88a7
      #define GL_PROGRAM_PARAMETERS_ARB                                   0x88a8
      #define GL_MAX_PROGRAM_PARAMETERS_ARB                               0x88a9
      #define GL_PROGRAM_NATIVE_PARAMETERS_ARB                            0x88aa
      #define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB                        0x88ab
      #define GL_PROGRAM_ATTRIBS_ARB                                      0x88ac
      #define GL_MAX_PROGRAM_ATTRIBS_ARB                                  0x88ad
      #define GL_PROGRAM_NATIVE_ATTRIBS_ARB                               0x88ae
      #define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB                           0x88af
      #define GL_PROGRAM_ADDRESS_REGISTERS_ARB                            0x88b0
      #define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB                        0x88b1
      #define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB                     0x88b2
      #define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB                 0x88b3
      #define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB                         0x88b4
      #define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB                           0x88b5
      #define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB                          0x88b6
      #define GL_TRANSPOSE_CURRENT_MATRIX_ARB                             0x88b7
      #define GL_MATRIX0_ARB                                              0x88c0
      #define GL_MATRIX1_ARB                                              0x88c1
      #define GL_MATRIX2_ARB                                              0x88c2
      #define GL_MATRIX3_ARB                                              0x88c3
      #define GL_MATRIX4_ARB                                              0x88c4
      #define GL_MATRIX5_ARB                                              0x88c5
      #define GL_MATRIX6_ARB                                              0x88c6
      #define GL_MATRIX7_ARB                                              0x88c7
      #define GL_MATRIX8_ARB                                              0x88c8
      #define GL_MATRIX9_ARB                                              0x88c9
      #define GL_MATRIX10_ARB                                             0x88ca
      #define GL_MATRIX11_ARB                                             0x88cb
      #define GL_MATRIX12_ARB                                             0x88cc
      #define GL_MATRIX13_ARB                                             0x88cd
      #define GL_MATRIX14_ARB                                             0x88ce
      #define GL_MATRIX15_ARB                                             0x88cf
      #define GL_MATRIX16_ARB                                             0x88d0
      #define GL_MATRIX17_ARB                                             0x88d1
      #define GL_MATRIX18_ARB                                             0x88d2
      #define GL_MATRIX19_ARB                                             0x88d3
      #define GL_MATRIX20_ARB                                             0x88d4
      #define GL_MATRIX21_ARB                                             0x88d5
      #define GL_MATRIX22_ARB                                             0x88d6
      #define GL_MATRIX23_ARB                                             0x88d7
      #define GL_MATRIX24_ARB                                             0x88d8
      #define GL_MATRIX25_ARB                                             0x88d9
      #define GL_MATRIX26_ARB                                             0x88da
      #define GL_MATRIX27_ARB                                             0x88db
      #define GL_MATRIX28_ARB                                             0x88dc
      #define GL_MATRIX29_ARB                                             0x88dd
      #define GL_MATRIX30_ARB                                             0x88de
      #define GL_MATRIX31_ARB                                             0x88df

   #endif /* GL_COLOR_SUM_ARB */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBindProgramARB                                 OGLEXT_MAKEGLNAME(BindProgramARB)
      #define glDeleteProgramsARB                              OGLEXT_MAKEGLNAME(DeleteProgramsARB)
      #define glDisableVertexAttribArrayARB                    OGLEXT_MAKEGLNAME(DisableVertexAttribArrayARB)
      #define glEnableVertexAttribArrayARB                     OGLEXT_MAKEGLNAME(EnableVertexAttribArrayARB)
      #define glGenProgramsARB                                 OGLEXT_MAKEGLNAME(GenProgramsARB)
      #define glGetProgramEnvParameterdvARB                    OGLEXT_MAKEGLNAME(GetProgramEnvParameterdvARB)
      #define glGetProgramEnvParameterfvARB                    OGLEXT_MAKEGLNAME(GetProgramEnvParameterfvARB)
      #define glGetProgramivARB                                OGLEXT_MAKEGLNAME(GetProgramivARB)
      #define glGetProgramLocalParameterdvARB                  OGLEXT_MAKEGLNAME(GetProgramLocalParameterdvARB)
      #define glGetProgramLocalParameterfvARB                  OGLEXT_MAKEGLNAME(GetProgramLocalParameterfvARB)
      #define glGetProgramStringARB                            OGLEXT_MAKEGLNAME(GetProgramStringARB)
      #define glGetVertexAttribdvARB                           OGLEXT_MAKEGLNAME(GetVertexAttribdvARB)
      #define glGetVertexAttribfvARB                           OGLEXT_MAKEGLNAME(GetVertexAttribfvARB)
      #define glGetVertexAttribivARB                           OGLEXT_MAKEGLNAME(GetVertexAttribivARB)
      #define glGetVertexAttribPointervARB                     OGLEXT_MAKEGLNAME(GetVertexAttribPointervARB)
      #define glIsProgramARB                                   OGLEXT_MAKEGLNAME(IsProgramARB)
      #define glProgramEnvParameter4dARB                       OGLEXT_MAKEGLNAME(ProgramEnvParameter4dARB)
      #define glProgramEnvParameter4dvARB                      OGLEXT_MAKEGLNAME(ProgramEnvParameter4dvARB)
      #define glProgramEnvParameter4fARB                       OGLEXT_MAKEGLNAME(ProgramEnvParameter4fARB)
      #define glProgramEnvParameter4fvARB                      OGLEXT_MAKEGLNAME(ProgramEnvParameter4fvARB)
      #define glProgramLocalParameter4dARB                     OGLEXT_MAKEGLNAME(ProgramLocalParameter4dARB)
      #define glProgramLocalParameter4dvARB                    OGLEXT_MAKEGLNAME(ProgramLocalParameter4dvARB)
      #define glProgramLocalParameter4fARB                     OGLEXT_MAKEGLNAME(ProgramLocalParameter4fARB)
      #define glProgramLocalParameter4fvARB                    OGLEXT_MAKEGLNAME(ProgramLocalParameter4fvARB)
      #define glProgramStringARB                               OGLEXT_MAKEGLNAME(ProgramStringARB)
      #define glVertexAttrib1dARB                              OGLEXT_MAKEGLNAME(VertexAttrib1dARB)
      #define glVertexAttrib1dvARB                             OGLEXT_MAKEGLNAME(VertexAttrib1dvARB)
      #define glVertexAttrib1fARB                              OGLEXT_MAKEGLNAME(VertexAttrib1fARB)
      #define glVertexAttrib1fvARB                             OGLEXT_MAKEGLNAME(VertexAttrib1fvARB)
      #define glVertexAttrib1sARB                              OGLEXT_MAKEGLNAME(VertexAttrib1sARB)
      #define glVertexAttrib1svARB                             OGLEXT_MAKEGLNAME(VertexAttrib1svARB)
      #define glVertexAttrib2dARB                              OGLEXT_MAKEGLNAME(VertexAttrib2dARB)
      #define glVertexAttrib2dvARB                             OGLEXT_MAKEGLNAME(VertexAttrib2dvARB)
      #define glVertexAttrib2fARB                              OGLEXT_MAKEGLNAME(VertexAttrib2fARB)
      #define glVertexAttrib2fvARB                             OGLEXT_MAKEGLNAME(VertexAttrib2fvARB)
      #define glVertexAttrib2sARB                              OGLEXT_MAKEGLNAME(VertexAttrib2sARB)
      #define glVertexAttrib2svARB                             OGLEXT_MAKEGLNAME(VertexAttrib2svARB)
      #define glVertexAttrib3dARB                              OGLEXT_MAKEGLNAME(VertexAttrib3dARB)
      #define glVertexAttrib3dvARB                             OGLEXT_MAKEGLNAME(VertexAttrib3dvARB)
      #define glVertexAttrib3fARB                              OGLEXT_MAKEGLNAME(VertexAttrib3fARB)
      #define glVertexAttrib3fvARB                             OGLEXT_MAKEGLNAME(VertexAttrib3fvARB)
      #define glVertexAttrib3sARB                              OGLEXT_MAKEGLNAME(VertexAttrib3sARB)
      #define glVertexAttrib3svARB                             OGLEXT_MAKEGLNAME(VertexAttrib3svARB)
      #define glVertexAttrib4bvARB                             OGLEXT_MAKEGLNAME(VertexAttrib4bvARB)
      #define glVertexAttrib4dARB                              OGLEXT_MAKEGLNAME(VertexAttrib4dARB)
      #define glVertexAttrib4dvARB                             OGLEXT_MAKEGLNAME(VertexAttrib4dvARB)
      #define glVertexAttrib4fARB                              OGLEXT_MAKEGLNAME(VertexAttrib4fARB)
      #define glVertexAttrib4fvARB                             OGLEXT_MAKEGLNAME(VertexAttrib4fvARB)
      #define glVertexAttrib4ivARB                             OGLEXT_MAKEGLNAME(VertexAttrib4ivARB)
      #define glVertexAttrib4NbvARB                            OGLEXT_MAKEGLNAME(VertexAttrib4NbvARB)
      #define glVertexAttrib4NivARB                            OGLEXT_MAKEGLNAME(VertexAttrib4NivARB)
      #define glVertexAttrib4NsvARB                            OGLEXT_MAKEGLNAME(VertexAttrib4NsvARB)
      #define glVertexAttrib4NubARB                            OGLEXT_MAKEGLNAME(VertexAttrib4NubARB)
      #define glVertexAttrib4NubvARB                           OGLEXT_MAKEGLNAME(VertexAttrib4NubvARB)
      #define glVertexAttrib4NuivARB                           OGLEXT_MAKEGLNAME(VertexAttrib4NuivARB)
      #define glVertexAttrib4NusvARB                           OGLEXT_MAKEGLNAME(VertexAttrib4NusvARB)
      #define glVertexAttrib4sARB                              OGLEXT_MAKEGLNAME(VertexAttrib4sARB)
      #define glVertexAttrib4svARB                             OGLEXT_MAKEGLNAME(VertexAttrib4svARB)
      #define glVertexAttrib4ubvARB                            OGLEXT_MAKEGLNAME(VertexAttrib4ubvARB)
      #define glVertexAttrib4uivARB                            OGLEXT_MAKEGLNAME(VertexAttrib4uivARB)
      #define glVertexAttrib4usvARB                            OGLEXT_MAKEGLNAME(VertexAttrib4usvARB)
      #define glVertexAttribPointerARB                         OGLEXT_MAKEGLNAME(VertexAttribPointerARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBindProgramARB(GLenum, GLuint);
      GLAPI GLvoid            glDeleteProgramsARB(GLsizei, GLuint const *);
      GLAPI GLvoid            glDisableVertexAttribArrayARB(GLuint);
      GLAPI GLvoid            glEnableVertexAttribArrayARB(GLuint);
      GLAPI GLvoid            glGenProgramsARB(GLsizei, GLuint *);
      GLAPI GLvoid            glGetProgramEnvParameterdvARB(GLenum, GLuint, GLdouble *);
      GLAPI GLvoid            glGetProgramEnvParameterfvARB(GLenum, GLuint, GLfloat *);
      GLAPI GLvoid            glGetProgramivARB(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetProgramLocalParameterdvARB(GLenum, GLuint, GLdouble *);
      GLAPI GLvoid            glGetProgramLocalParameterfvARB(GLenum, GLuint, GLfloat *);
      GLAPI GLvoid            glGetProgramStringARB(GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetVertexAttribdvARB(GLuint, GLenum, GLdouble *);
      GLAPI GLvoid            glGetVertexAttribfvARB(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetVertexAttribivARB(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetVertexAttribPointervARB(GLuint, GLenum, GLvoid * *);
      GLAPI GLboolean         glIsProgramARB(GLuint);
      GLAPI GLvoid            glProgramEnvParameter4dARB(GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glProgramEnvParameter4dvARB(GLenum, GLuint, GLdouble const *);
      GLAPI GLvoid            glProgramEnvParameter4fARB(GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glProgramEnvParameter4fvARB(GLenum, GLuint, GLfloat const *);
      GLAPI GLvoid            glProgramLocalParameter4dARB(GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glProgramLocalParameter4dvARB(GLenum, GLuint, GLdouble const *);
      GLAPI GLvoid            glProgramLocalParameter4fARB(GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glProgramLocalParameter4fvARB(GLenum, GLuint, GLfloat const *);
      GLAPI GLvoid            glProgramStringARB(GLenum, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glVertexAttrib1dARB(GLuint, GLdouble);
      GLAPI GLvoid            glVertexAttrib1dvARB(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib1fARB(GLuint, GLfloat);
      GLAPI GLvoid            glVertexAttrib1fvARB(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib1sARB(GLuint, GLshort);
      GLAPI GLvoid            glVertexAttrib1svARB(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib2dARB(GLuint, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexAttrib2dvARB(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib2fARB(GLuint, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexAttrib2fvARB(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib2sARB(GLuint, GLshort, GLshort);
      GLAPI GLvoid            glVertexAttrib2svARB(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib3dARB(GLuint, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexAttrib3dvARB(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib3fARB(GLuint, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexAttrib3fvARB(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib3sARB(GLuint, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glVertexAttrib3svARB(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib4bvARB(GLuint, GLbyte const *);
      GLAPI GLvoid            glVertexAttrib4dARB(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexAttrib4dvARB(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib4fARB(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexAttrib4fvARB(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib4ivARB(GLuint, GLint const *);
      GLAPI GLvoid            glVertexAttrib4NbvARB(GLuint, GLbyte const *);
      GLAPI GLvoid            glVertexAttrib4NivARB(GLuint, GLint const *);
      GLAPI GLvoid            glVertexAttrib4NsvARB(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib4NubARB(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
      GLAPI GLvoid            glVertexAttrib4NubvARB(GLuint, GLubyte const *);
      GLAPI GLvoid            glVertexAttrib4NuivARB(GLuint, GLuint const *);
      GLAPI GLvoid            glVertexAttrib4NusvARB(GLuint, GLushort const *);
      GLAPI GLvoid            glVertexAttrib4sARB(GLuint, GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glVertexAttrib4svARB(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib4ubvARB(GLuint, GLubyte const *);
      GLAPI GLvoid            glVertexAttrib4uivARB(GLuint, GLuint const *);
      GLAPI GLvoid            glVertexAttrib4usvARB(GLuint, GLushort const *);
      GLAPI GLvoid            glVertexAttribPointerARB(GLuint, GLint, GLenum, GLboolean, GLsizei, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_vertex_program */


/* ---[ GL_ARB_window_pos ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ARB/window_pos.txt */

#ifndef GL_ARB_window_pos

   #define GL_ARB_window_pos 1
   #define GL_ARB_window_pos_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glWindowPos2dARB                                 OGLEXT_MAKEGLNAME(WindowPos2dARB)
      #define glWindowPos2dvARB                                OGLEXT_MAKEGLNAME(WindowPos2dvARB)
      #define glWindowPos2fARB                                 OGLEXT_MAKEGLNAME(WindowPos2fARB)
      #define glWindowPos2fvARB                                OGLEXT_MAKEGLNAME(WindowPos2fvARB)
      #define glWindowPos2iARB                                 OGLEXT_MAKEGLNAME(WindowPos2iARB)
      #define glWindowPos2ivARB                                OGLEXT_MAKEGLNAME(WindowPos2ivARB)
      #define glWindowPos2sARB                                 OGLEXT_MAKEGLNAME(WindowPos2sARB)
      #define glWindowPos2svARB                                OGLEXT_MAKEGLNAME(WindowPos2svARB)
      #define glWindowPos3dARB                                 OGLEXT_MAKEGLNAME(WindowPos3dARB)
      #define glWindowPos3dvARB                                OGLEXT_MAKEGLNAME(WindowPos3dvARB)
      #define glWindowPos3fARB                                 OGLEXT_MAKEGLNAME(WindowPos3fARB)
      #define glWindowPos3fvARB                                OGLEXT_MAKEGLNAME(WindowPos3fvARB)
      #define glWindowPos3iARB                                 OGLEXT_MAKEGLNAME(WindowPos3iARB)
      #define glWindowPos3ivARB                                OGLEXT_MAKEGLNAME(WindowPos3ivARB)
      #define glWindowPos3sARB                                 OGLEXT_MAKEGLNAME(WindowPos3sARB)
      #define glWindowPos3svARB                                OGLEXT_MAKEGLNAME(WindowPos3svARB)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glWindowPos2dARB(GLdouble, GLdouble);
      GLAPI GLvoid            glWindowPos2dvARB(GLdouble const *);
      GLAPI GLvoid            glWindowPos2fARB(GLfloat, GLfloat);
      GLAPI GLvoid            glWindowPos2fvARB(GLfloat const *);
      GLAPI GLvoid            glWindowPos2iARB(GLint, GLint);
      GLAPI GLvoid            glWindowPos2ivARB(GLint const *);
      GLAPI GLvoid            glWindowPos2sARB(GLshort, GLshort);
      GLAPI GLvoid            glWindowPos2svARB(GLshort const *);
      GLAPI GLvoid            glWindowPos3dARB(GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glWindowPos3dvARB(GLdouble const *);
      GLAPI GLvoid            glWindowPos3fARB(GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glWindowPos3fvARB(GLfloat const *);
      GLAPI GLvoid            glWindowPos3iARB(GLint, GLint, GLint);
      GLAPI GLvoid            glWindowPos3ivARB(GLint const *);
      GLAPI GLvoid            glWindowPos3sARB(GLshort, GLshort, GLshort);
      GLAPI GLvoid            glWindowPos3svARB(GLshort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ARB_window_pos */


/* ---[ GL_ATI_draw_buffers ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/draw_buffers.txt */

#ifndef GL_ATI_draw_buffers

   #define GL_ATI_draw_buffers 1
   #define GL_ATI_draw_buffers_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MAX_DRAW_BUFFERS_ATI

      #define GL_MAX_DRAW_BUFFERS_ATI                                     0x8824
      #define GL_DRAW_BUFFER0_ATI                                         0x8825
      #define GL_DRAW_BUFFER1_ATI                                         0x8826
      #define GL_DRAW_BUFFER2_ATI                                         0x8827
      #define GL_DRAW_BUFFER3_ATI                                         0x8828
      #define GL_DRAW_BUFFER4_ATI                                         0x8829
      #define GL_DRAW_BUFFER5_ATI                                         0x882a
      #define GL_DRAW_BUFFER6_ATI                                         0x882b
      #define GL_DRAW_BUFFER7_ATI                                         0x882c
      #define GL_DRAW_BUFFER8_ATI                                         0x882d
      #define GL_DRAW_BUFFER9_ATI                                         0x882e
      #define GL_DRAW_BUFFER10_ATI                                        0x882f
      #define GL_DRAW_BUFFER11_ATI                                        0x8830
      #define GL_DRAW_BUFFER12_ATI                                        0x8831
      #define GL_DRAW_BUFFER13_ATI                                        0x8832
      #define GL_DRAW_BUFFER14_ATI                                        0x8833
      #define GL_DRAW_BUFFER15_ATI                                        0x8834

   #endif /* GL_MAX_DRAW_BUFFERS_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDrawBuffersATI                                 OGLEXT_MAKEGLNAME(DrawBuffersATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDrawBuffersATI(GLsizei, GLenum const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_draw_buffers */


/* ---[ GL_ATI_element_array ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/element_array.txt */

#ifndef GL_ATI_element_array

   #define GL_ATI_element_array 1
   #define GL_ATI_element_array_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ELEMENT_ARRAY_ATI

      #define GL_ELEMENT_ARRAY_ATI                                        0x8768
      #define GL_ELEMENT_ARRAY_TYPE_ATI                                   0x8769
      #define GL_ELEMENT_ARRAY_POINTER_ATI                                0x876a

   #endif /* GL_ELEMENT_ARRAY_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDrawElementArrayATI                            OGLEXT_MAKEGLNAME(DrawElementArrayATI)
      #define glDrawRangeElementArrayATI                       OGLEXT_MAKEGLNAME(DrawRangeElementArrayATI)
      #define glElementPointerATI                              OGLEXT_MAKEGLNAME(ElementPointerATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDrawElementArrayATI(GLenum, GLsizei);
      GLAPI GLvoid            glDrawRangeElementArrayATI(GLenum, GLuint, GLuint, GLsizei);
      GLAPI GLvoid            glElementPointerATI(GLenum, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_element_array */


/* ---[ GL_ATI_envmap_bumpmap ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/envmap_bumpmap.txt */

#ifndef GL_ATI_envmap_bumpmap

   #define GL_ATI_envmap_bumpmap 1
   #define GL_ATI_envmap_bumpmap_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_BUMP_ROT_MATRIX_ATI

      #define GL_BUMP_ROT_MATRIX_ATI                                      0x8775
      #define GL_BUMP_ROT_MATRIX_SIZE_ATI                                 0x8776
      #define GL_BUMP_NUM_TEX_UNITS_ATI                                   0x8777
      #define GL_BUMP_TEX_UNITS_ATI                                       0x8778
      #define GL_DUDV_ATI                                                 0x8779
      #define GL_DU8DV8_ATI                                               0x877a
      #define GL_BUMP_ENVMAP_ATI                                          0x877b
      #define GL_BUMP_TARGET_ATI                                          0x877c

   #endif /* GL_BUMP_ROT_MATRIX_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetTexBumpParameterfvATI                       OGLEXT_MAKEGLNAME(GetTexBumpParameterfvATI)
      #define glGetTexBumpParameterivATI                       OGLEXT_MAKEGLNAME(GetTexBumpParameterivATI)
      #define glTexBumpParameterfvATI                          OGLEXT_MAKEGLNAME(TexBumpParameterfvATI)
      #define glTexBumpParameterivATI                          OGLEXT_MAKEGLNAME(TexBumpParameterivATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetTexBumpParameterfvATI(GLenum, GLfloat *);
      GLAPI GLvoid            glGetTexBumpParameterivATI(GLenum, GLint *);
      GLAPI GLvoid            glTexBumpParameterfvATI(GLenum, GLfloat const *);
      GLAPI GLvoid            glTexBumpParameterivATI(GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_envmap_bumpmap */


/* ---[ GL_ATI_fragment_shader ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/fragment_shader.txt */

#ifndef GL_ATI_fragment_shader

   #define GL_ATI_fragment_shader 1
   #define GL_ATI_fragment_shader_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_RED_BIT_ATI

      #define GL_RED_BIT_ATI                                              0x1
      #define GL_2X_BIT_ATI                                               0x1
      #define GL_GREEN_BIT_ATI                                            0x2
      #define GL_4X_BIT_ATI                                               0x2
      #define GL_COMP_BIT_ATI                                             0x2
      #define GL_BLUE_BIT_ATI                                             0x4
      #define GL_8X_BIT_ATI                                               0x4
      #define GL_NEGATE_BIT_ATI                                           0x4
      #define GL_HALF_BIT_ATI                                             0x8
      #define GL_BIAS_BIT_ATI                                             0x8
      #define GL_QUARTER_BIT_ATI                                          0x10
      #define GL_EIGHTH_BIT_ATI                                           0x20
      #define GL_SATURATE_BIT_ATI                                         0x40
      #define GL_FRAGMENT_SHADER_ATI                                      0x8920
      #define GL_REG_0_ATI                                                0x8921
      #define GL_REG_1_ATI                                                0x8922
      #define GL_REG_2_ATI                                                0x8923
      #define GL_REG_3_ATI                                                0x8924
      #define GL_REG_4_ATI                                                0x8925
      #define GL_REG_5_ATI                                                0x8926
      #define GL_REG_6_ATI                                                0x8927
      #define GL_REG_7_ATI                                                0x8928
      #define GL_REG_8_ATI                                                0x8929
      #define GL_REG_9_ATI                                                0x892a
      #define GL_REG_10_ATI                                               0x892b
      #define GL_REG_11_ATI                                               0x892c
      #define GL_REG_12_ATI                                               0x892d
      #define GL_REG_13_ATI                                               0x892e
      #define GL_REG_14_ATI                                               0x892f
      #define GL_REG_15_ATI                                               0x8930
      #define GL_REG_16_ATI                                               0x8931
      #define GL_REG_17_ATI                                               0x8932
      #define GL_REG_18_ATI                                               0x8933
      #define GL_REG_19_ATI                                               0x8934
      #define GL_REG_20_ATI                                               0x8935
      #define GL_REG_21_ATI                                               0x8936
      #define GL_REG_22_ATI                                               0x8937
      #define GL_REG_23_ATI                                               0x8938
      #define GL_REG_24_ATI                                               0x8939
      #define GL_REG_25_ATI                                               0x893a
      #define GL_REG_26_ATI                                               0x893b
      #define GL_REG_27_ATI                                               0x893c
      #define GL_REG_28_ATI                                               0x893d
      #define GL_REG_29_ATI                                               0x893e
      #define GL_REG_30_ATI                                               0x893f
      #define GL_REG_31_ATI                                               0x8940
      #define GL_CON_0_ATI                                                0x8941
      #define GL_CON_1_ATI                                                0x8942
      #define GL_CON_2_ATI                                                0x8943
      #define GL_CON_3_ATI                                                0x8944
      #define GL_CON_4_ATI                                                0x8945
      #define GL_CON_5_ATI                                                0x8946
      #define GL_CON_6_ATI                                                0x8947
      #define GL_CON_7_ATI                                                0x8948
      #define GL_CON_8_ATI                                                0x8949
      #define GL_CON_9_ATI                                                0x894a
      #define GL_CON_10_ATI                                               0x894b
      #define GL_CON_11_ATI                                               0x894c
      #define GL_CON_12_ATI                                               0x894d
      #define GL_CON_13_ATI                                               0x894e
      #define GL_CON_14_ATI                                               0x894f
      #define GL_CON_15_ATI                                               0x8950
      #define GL_CON_16_ATI                                               0x8951
      #define GL_CON_17_ATI                                               0x8952
      #define GL_CON_18_ATI                                               0x8953
      #define GL_CON_19_ATI                                               0x8954
      #define GL_CON_20_ATI                                               0x8955
      #define GL_CON_21_ATI                                               0x8956
      #define GL_CON_22_ATI                                               0x8957
      #define GL_CON_23_ATI                                               0x8958
      #define GL_CON_24_ATI                                               0x8959
      #define GL_CON_25_ATI                                               0x895a
      #define GL_CON_26_ATI                                               0x895b
      #define GL_CON_27_ATI                                               0x895c
      #define GL_CON_28_ATI                                               0x895d
      #define GL_CON_29_ATI                                               0x895e
      #define GL_CON_30_ATI                                               0x895f
      #define GL_CON_31_ATI                                               0x8960
      #define GL_MOV_ATI                                                  0x8961
      #define GL_ADD_ATI                                                  0x8963
      #define GL_MUL_ATI                                                  0x8964
      #define GL_SUB_ATI                                                  0x8965
      #define GL_DOT3_ATI                                                 0x8966
      #define GL_DOT4_ATI                                                 0x8967
      #define GL_MAD_ATI                                                  0x8968
      #define GL_LERP_ATI                                                 0x8969
      #define GL_CND_ATI                                                  0x896a
      #define GL_CND0_ATI                                                 0x896b
      #define GL_DOT2_ADD_ATI                                             0x896c
      #define GL_SECONDARY_INTERPOLATOR_ATI                               0x896d
      #define GL_NUM_FRAGMENT_REGISTERS_ATI                               0x896e
      #define GL_NUM_FRAGMENT_CONSTANTS_ATI                               0x896f
      #define GL_NUM_PASSES_ATI                                           0x8970
      #define GL_NUM_INSTRUCTIONS_PER_PASS_ATI                            0x8971
      #define GL_NUM_INSTRUCTIONS_TOTAL_ATI                               0x8972
      #define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI                    0x8973
      #define GL_NUM_LOOPBACK_COMPONENTS_ATI                              0x8974
      #define GL_COLOR_ALPHA_PAIRING_ATI                                  0x8975
      #define GL_SWIZZLE_STR_ATI                                          0x8976
      #define GL_SWIZZLE_STQ_ATI                                          0x8977
      #define GL_SWIZZLE_STR_DR_ATI                                       0x8978
      #define GL_SWIZZLE_STQ_DQ_ATI                                       0x8979
      #define GL_SWIZZLE_STRQ_ATI                                         0x897a
      #define GL_SWIZZLE_STRQ_DQ_ATI                                      0x897b

   #endif /* GL_RED_BIT_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glAlphaFragmentOp1ATI                            OGLEXT_MAKEGLNAME(AlphaFragmentOp1ATI)
      #define glAlphaFragmentOp2ATI                            OGLEXT_MAKEGLNAME(AlphaFragmentOp2ATI)
      #define glAlphaFragmentOp3ATI                            OGLEXT_MAKEGLNAME(AlphaFragmentOp3ATI)
      #define glBeginFragmentShaderATI                         OGLEXT_MAKEGLNAME(BeginFragmentShaderATI)
      #define glBindFragmentShaderATI                          OGLEXT_MAKEGLNAME(BindFragmentShaderATI)
      #define glColorFragmentOp1ATI                            OGLEXT_MAKEGLNAME(ColorFragmentOp1ATI)
      #define glColorFragmentOp2ATI                            OGLEXT_MAKEGLNAME(ColorFragmentOp2ATI)
      #define glColorFragmentOp3ATI                            OGLEXT_MAKEGLNAME(ColorFragmentOp3ATI)
      #define glDeleteFragmentShaderATI                        OGLEXT_MAKEGLNAME(DeleteFragmentShaderATI)
      #define glEndFragmentShaderATI                           OGLEXT_MAKEGLNAME(EndFragmentShaderATI)
      #define glGenFragmentShadersATI                          OGLEXT_MAKEGLNAME(GenFragmentShadersATI)
      #define glPassTexCoordATI                                OGLEXT_MAKEGLNAME(PassTexCoordATI)
      #define glSampleMapATI                                   OGLEXT_MAKEGLNAME(SampleMapATI)
      #define glSetFragmentShaderConstantATI                   OGLEXT_MAKEGLNAME(SetFragmentShaderConstantATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glAlphaFragmentOp1ATI(GLenum, GLuint, GLuint, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glAlphaFragmentOp2ATI(GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glAlphaFragmentOp3ATI(GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glBeginFragmentShaderATI();
      GLAPI GLvoid            glBindFragmentShaderATI(GLuint);
      GLAPI GLvoid            glColorFragmentOp1ATI(GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glColorFragmentOp2ATI(GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glColorFragmentOp3ATI(GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glDeleteFragmentShaderATI(GLuint);
      GLAPI GLvoid            glEndFragmentShaderATI();
      GLAPI GLuint            glGenFragmentShadersATI(GLuint);
      GLAPI GLvoid            glPassTexCoordATI(GLuint, GLuint, GLenum);
      GLAPI GLvoid            glSampleMapATI(GLuint, GLuint, GLenum);
      GLAPI GLvoid            glSetFragmentShaderConstantATI(GLuint, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_fragment_shader */


/* ---[ GL_ATI_map_object_buffer ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/map_object_buffer.txt */

#ifndef GL_ATI_map_object_buffer

   #define GL_ATI_map_object_buffer 1
   #define GL_ATI_map_object_buffer_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glMapObjectBufferATI                             OGLEXT_MAKEGLNAME(MapObjectBufferATI)
      #define glUnmapObjectBufferATI                           OGLEXT_MAKEGLNAME(UnmapObjectBufferATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid *          glMapObjectBufferATI(GLuint);
      GLAPI GLvoid            glUnmapObjectBufferATI(GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_map_object_buffer */


/* ---[ GL_ATI_pn_triangles ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/pn_triangles.txt */

#ifndef GL_ATI_pn_triangles

   #define GL_ATI_pn_triangles 1
   #define GL_ATI_pn_triangles_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PN_TRIANGLES_ATI

      #define GL_PN_TRIANGLES_ATI                                         0x87f0
      #define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI                   0x87f1
      #define GL_PN_TRIANGLES_POINT_MODE_ATI                              0x87f2
      #define GL_PN_TRIANGLES_NORMAL_MODE_ATI                             0x87f3
      #define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI                       0x87f4
      #define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI                       0x87f5
      #define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI                        0x87f6
      #define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI                      0x87f7
      #define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI                   0x87f8

   #endif /* GL_PN_TRIANGLES_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPNTrianglesfATI                                OGLEXT_MAKEGLNAME(PNTrianglesfATI)
      #define glPNTrianglesiATI                                OGLEXT_MAKEGLNAME(PNTrianglesiATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPNTrianglesfATI(GLenum, GLfloat);
      GLAPI GLvoid            glPNTrianglesiATI(GLenum, GLint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_pn_triangles */


/* ---[ GL_ATI_separate_stencil ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/separate_stencil.txt */

#ifndef GL_ATI_separate_stencil

   #define GL_ATI_separate_stencil 1
   #define GL_ATI_separate_stencil_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_STENCIL_BACK_FUNC_ATI

      #define GL_STENCIL_BACK_FUNC_ATI                                    0x8800
      #define GL_STENCIL_BACK_FAIL_ATI                                    0x8801
      #define GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI                         0x8802
      #define GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI                         0x8803

   #endif /* GL_STENCIL_BACK_FUNC_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glStencilFuncSeparateATI                         OGLEXT_MAKEGLNAME(StencilFuncSeparateATI)
      #define glStencilOpSeparateATI                           OGLEXT_MAKEGLNAME(StencilOpSeparateATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glStencilFuncSeparateATI(GLenum, GLenum, GLint, GLuint);
      GLAPI GLvoid            glStencilOpSeparateATI(GLenum, GLenum, GLenum, GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_separate_stencil */


/* ---[ GL_ATI_texture_env_combine3 ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/texture_env_combine3.txt */

#ifndef GL_ATI_texture_env_combine3

   #define GL_ATI_texture_env_combine3 1
   #define GL_ATI_texture_env_combine3_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MODULATE_ADD_ATI

      #define GL_MODULATE_ADD_ATI                                         0x8744
      #define GL_MODULATE_SIGNED_ADD_ATI                                  0x8745
      #define GL_MODULATE_SUBTRACT_ATI                                    0x8746

   #endif /* GL_MODULATE_ADD_ATI */

#endif /* GL_ATI_texture_env_combine3 */


/* ---[ GL_ATI_texture_float ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/texture_float.txt */

#ifndef GL_ATI_texture_float

   #define GL_ATI_texture_float 1
   #define GL_ATI_texture_float_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_RGBA_FLOAT32_ATI

      #define GL_RGBA_FLOAT32_ATI                                         0x8814
      #define GL_RGB_FLOAT32_ATI                                          0x8815
      #define GL_ALPHA_FLOAT32_ATI                                        0x8816
      #define GL_INTENSITY_FLOAT32_ATI                                    0x8817
      #define GL_LUMINANCE_FLOAT32_ATI                                    0x8818
      #define GL_LUMINANCE_ALPHA_FLOAT32_ATI                              0x8819
      #define GL_RGBA_FLOAT16_ATI                                         0x881a
      #define GL_RGB_FLOAT16_ATI                                          0x881b
      #define GL_ALPHA_FLOAT16_ATI                                        0x881c
      #define GL_INTENSITY_FLOAT16_ATI                                    0x881d
      #define GL_LUMINANCE_FLOAT16_ATI                                    0x881e
      #define GL_LUMINANCE_ALPHA_FLOAT16_ATI                              0x881f

   #endif /* GL_RGBA_FLOAT32_ATI */

#endif /* GL_ATI_texture_float */


/* ---[ GL_ATI_texture_mirror_once ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/texture_mirror_once.txt */

#ifndef GL_ATI_texture_mirror_once

   #define GL_ATI_texture_mirror_once 1
   #define GL_ATI_texture_mirror_once_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MIRROR_CLAMP_ATI

      #define GL_MIRROR_CLAMP_ATI                                         0x8742
      #define GL_MIRROR_CLAMP_TO_EDGE_ATI                                 0x8743

   #endif /* GL_MIRROR_CLAMP_ATI */

#endif /* GL_ATI_texture_mirror_once */


/* ---[ GL_ATI_text_fragment_shader ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/text_fragment_shader.txt */

#ifndef GL_ATI_text_fragment_shader

   #define GL_ATI_text_fragment_shader 1
   #define GL_ATI_text_fragment_shader_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXT_FRAGMENT_SHADER_ATI

      #define GL_TEXT_FRAGMENT_SHADER_ATI                                 0x8200

   #endif /* GL_TEXT_FRAGMENT_SHADER_ATI */

#endif /* GL_ATI_text_fragment_shader */


/* ---[ GL_ATI_vertex_array_object ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/vertex_array_object.txt */

#ifndef GL_ATI_vertex_array_object

   #define GL_ATI_vertex_array_object 1
   #define GL_ATI_vertex_array_object_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_STATIC_ATI

      #define GL_STATIC_ATI                                               0x8760
      #define GL_DYNAMIC_ATI                                              0x8761
      #define GL_PRESERVE_ATI                                             0x8762
      #define GL_DISCARD_ATI                                              0x8763
      #define GL_OBJECT_BUFFER_SIZE_ATI                                   0x8764
      #define GL_OBJECT_BUFFER_USAGE_ATI                                  0x8765
      #define GL_ARRAY_OBJECT_BUFFER_ATI                                  0x8766
      #define GL_ARRAY_OBJECT_OFFSET_ATI                                  0x8767

   #endif /* GL_STATIC_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glArrayObjectATI                                 OGLEXT_MAKEGLNAME(ArrayObjectATI)
      #define glFreeObjectBufferATI                            OGLEXT_MAKEGLNAME(FreeObjectBufferATI)
      #define glGetArrayObjectfvATI                            OGLEXT_MAKEGLNAME(GetArrayObjectfvATI)
      #define glGetArrayObjectivATI                            OGLEXT_MAKEGLNAME(GetArrayObjectivATI)
      #define glGetObjectBufferfvATI                           OGLEXT_MAKEGLNAME(GetObjectBufferfvATI)
      #define glGetObjectBufferivATI                           OGLEXT_MAKEGLNAME(GetObjectBufferivATI)
      #define glGetVariantArrayObjectfvATI                     OGLEXT_MAKEGLNAME(GetVariantArrayObjectfvATI)
      #define glGetVariantArrayObjectivATI                     OGLEXT_MAKEGLNAME(GetVariantArrayObjectivATI)
      #define glIsObjectBufferATI                              OGLEXT_MAKEGLNAME(IsObjectBufferATI)
      #define glNewObjectBufferATI                             OGLEXT_MAKEGLNAME(NewObjectBufferATI)
      #define glUpdateObjectBufferATI                          OGLEXT_MAKEGLNAME(UpdateObjectBufferATI)
      #define glVariantArrayObjectATI                          OGLEXT_MAKEGLNAME(VariantArrayObjectATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glArrayObjectATI(GLenum, GLint, GLenum, GLsizei, GLuint, GLuint);
      GLAPI GLvoid            glFreeObjectBufferATI(GLuint);
      GLAPI GLvoid            glGetArrayObjectfvATI(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetArrayObjectivATI(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetObjectBufferfvATI(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetObjectBufferivATI(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetVariantArrayObjectfvATI(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetVariantArrayObjectivATI(GLuint, GLenum, GLint *);
      GLAPI GLboolean         glIsObjectBufferATI(GLuint);
      GLAPI GLuint            glNewObjectBufferATI(GLsizei, GLvoid const *, GLenum);
      GLAPI GLvoid            glUpdateObjectBufferATI(GLuint, GLuint, GLsizei, GLvoid const *, GLenum);
      GLAPI GLvoid            glVariantArrayObjectATI(GLuint, GLenum, GLsizei, GLuint, GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_vertex_array_object */


/* ---[ GL_ATI_vertex_attrib_array_object ]------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/vertex_attrib_array_object.txt */

#ifndef GL_ATI_vertex_attrib_array_object

   #define GL_ATI_vertex_attrib_array_object 1
   #define GL_ATI_vertex_attrib_array_object_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetVertexAttribArrayObjectfvATI                OGLEXT_MAKEGLNAME(GetVertexAttribArrayObjectfvATI)
      #define glGetVertexAttribArrayObjectivATI                OGLEXT_MAKEGLNAME(GetVertexAttribArrayObjectivATI)
      #define glVertexAttribArrayObjectATI                     OGLEXT_MAKEGLNAME(VertexAttribArrayObjectATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetVertexAttribArrayObjectfvATI(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetVertexAttribArrayObjectivATI(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glVertexAttribArrayObjectATI(GLuint, GLint, GLenum, GLboolean, GLsizei, GLuint, GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_vertex_attrib_array_object */


/* ---[ GL_ATI_vertex_streams ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/ATI/vertex_streams.txt */

#ifndef GL_ATI_vertex_streams

   #define GL_ATI_vertex_streams 1
   #define GL_ATI_vertex_streams_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MAX_VERTEX_STREAMS_ATI

      #define GL_MAX_VERTEX_STREAMS_ATI                                   0x876b
      #define GL_VERTEX_STREAM0_ATI                                       0x876c
      #define GL_VERTEX_STREAM1_ATI                                       0x876d
      #define GL_VERTEX_STREAM2_ATI                                       0x876e
      #define GL_VERTEX_STREAM3_ATI                                       0x876f
      #define GL_VERTEX_STREAM4_ATI                                       0x8770
      #define GL_VERTEX_STREAM5_ATI                                       0x8771
      #define GL_VERTEX_STREAM6_ATI                                       0x8772
      #define GL_VERTEX_STREAM7_ATI                                       0x8773
      #define GL_VERTEX_SOURCE_ATI                                        0x8774

   #endif /* GL_MAX_VERTEX_STREAMS_ATI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glClientActiveVertexStreamATI                    OGLEXT_MAKEGLNAME(ClientActiveVertexStreamATI)
      #define glNormalStream3bATI                              OGLEXT_MAKEGLNAME(NormalStream3bATI)
      #define glNormalStream3bvATI                             OGLEXT_MAKEGLNAME(NormalStream3bvATI)
      #define glNormalStream3dATI                              OGLEXT_MAKEGLNAME(NormalStream3dATI)
      #define glNormalStream3dvATI                             OGLEXT_MAKEGLNAME(NormalStream3dvATI)
      #define glNormalStream3fATI                              OGLEXT_MAKEGLNAME(NormalStream3fATI)
      #define glNormalStream3fvATI                             OGLEXT_MAKEGLNAME(NormalStream3fvATI)
      #define glNormalStream3iATI                              OGLEXT_MAKEGLNAME(NormalStream3iATI)
      #define glNormalStream3ivATI                             OGLEXT_MAKEGLNAME(NormalStream3ivATI)
      #define glNormalStream3sATI                              OGLEXT_MAKEGLNAME(NormalStream3sATI)
      #define glNormalStream3svATI                             OGLEXT_MAKEGLNAME(NormalStream3svATI)
      #define glVertexBlendEnvfATI                             OGLEXT_MAKEGLNAME(VertexBlendEnvfATI)
      #define glVertexBlendEnviATI                             OGLEXT_MAKEGLNAME(VertexBlendEnviATI)
      #define glVertexStream1dATI                              OGLEXT_MAKEGLNAME(VertexStream1dATI)
      #define glVertexStream1dvATI                             OGLEXT_MAKEGLNAME(VertexStream1dvATI)
      #define glVertexStream1fATI                              OGLEXT_MAKEGLNAME(VertexStream1fATI)
      #define glVertexStream1fvATI                             OGLEXT_MAKEGLNAME(VertexStream1fvATI)
      #define glVertexStream1iATI                              OGLEXT_MAKEGLNAME(VertexStream1iATI)
      #define glVertexStream1ivATI                             OGLEXT_MAKEGLNAME(VertexStream1ivATI)
      #define glVertexStream1sATI                              OGLEXT_MAKEGLNAME(VertexStream1sATI)
      #define glVertexStream1svATI                             OGLEXT_MAKEGLNAME(VertexStream1svATI)
      #define glVertexStream2dATI                              OGLEXT_MAKEGLNAME(VertexStream2dATI)
      #define glVertexStream2dvATI                             OGLEXT_MAKEGLNAME(VertexStream2dvATI)
      #define glVertexStream2fATI                              OGLEXT_MAKEGLNAME(VertexStream2fATI)
      #define glVertexStream2fvATI                             OGLEXT_MAKEGLNAME(VertexStream2fvATI)
      #define glVertexStream2iATI                              OGLEXT_MAKEGLNAME(VertexStream2iATI)
      #define glVertexStream2ivATI                             OGLEXT_MAKEGLNAME(VertexStream2ivATI)
      #define glVertexStream2sATI                              OGLEXT_MAKEGLNAME(VertexStream2sATI)
      #define glVertexStream2svATI                             OGLEXT_MAKEGLNAME(VertexStream2svATI)
      #define glVertexStream3dATI                              OGLEXT_MAKEGLNAME(VertexStream3dATI)
      #define glVertexStream3dvATI                             OGLEXT_MAKEGLNAME(VertexStream3dvATI)
      #define glVertexStream3fATI                              OGLEXT_MAKEGLNAME(VertexStream3fATI)
      #define glVertexStream3fvATI                             OGLEXT_MAKEGLNAME(VertexStream3fvATI)
      #define glVertexStream3iATI                              OGLEXT_MAKEGLNAME(VertexStream3iATI)
      #define glVertexStream3ivATI                             OGLEXT_MAKEGLNAME(VertexStream3ivATI)
      #define glVertexStream3sATI                              OGLEXT_MAKEGLNAME(VertexStream3sATI)
      #define glVertexStream3svATI                             OGLEXT_MAKEGLNAME(VertexStream3svATI)
      #define glVertexStream4dATI                              OGLEXT_MAKEGLNAME(VertexStream4dATI)
      #define glVertexStream4dvATI                             OGLEXT_MAKEGLNAME(VertexStream4dvATI)
      #define glVertexStream4fATI                              OGLEXT_MAKEGLNAME(VertexStream4fATI)
      #define glVertexStream4fvATI                             OGLEXT_MAKEGLNAME(VertexStream4fvATI)
      #define glVertexStream4iATI                              OGLEXT_MAKEGLNAME(VertexStream4iATI)
      #define glVertexStream4ivATI                             OGLEXT_MAKEGLNAME(VertexStream4ivATI)
      #define glVertexStream4sATI                              OGLEXT_MAKEGLNAME(VertexStream4sATI)
      #define glVertexStream4svATI                             OGLEXT_MAKEGLNAME(VertexStream4svATI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glClientActiveVertexStreamATI(GLenum);
      GLAPI GLvoid            glNormalStream3bATI(GLenum, GLbyte, GLbyte, GLbyte);
      GLAPI GLvoid            glNormalStream3bvATI(GLenum, GLbyte const *);
      GLAPI GLvoid            glNormalStream3dATI(GLenum, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glNormalStream3dvATI(GLenum, GLdouble const *);
      GLAPI GLvoid            glNormalStream3fATI(GLenum, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glNormalStream3fvATI(GLenum, GLfloat const *);
      GLAPI GLvoid            glNormalStream3iATI(GLenum, GLint, GLint, GLint);
      GLAPI GLvoid            glNormalStream3ivATI(GLenum, GLint const *);
      GLAPI GLvoid            glNormalStream3sATI(GLenum, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glNormalStream3svATI(GLenum, GLshort const *);
      GLAPI GLvoid            glVertexBlendEnvfATI(GLenum, GLfloat);
      GLAPI GLvoid            glVertexBlendEnviATI(GLenum, GLint);
      GLAPI GLvoid            glVertexStream1dATI(GLenum, GLdouble);
      GLAPI GLvoid            glVertexStream1dvATI(GLenum, GLdouble const *);
      GLAPI GLvoid            glVertexStream1fATI(GLenum, GLfloat);
      GLAPI GLvoid            glVertexStream1fvATI(GLenum, GLfloat const *);
      GLAPI GLvoid            glVertexStream1iATI(GLenum, GLint);
      GLAPI GLvoid            glVertexStream1ivATI(GLenum, GLint const *);
      GLAPI GLvoid            glVertexStream1sATI(GLenum, GLshort);
      GLAPI GLvoid            glVertexStream1svATI(GLenum, GLshort const *);
      GLAPI GLvoid            glVertexStream2dATI(GLenum, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexStream2dvATI(GLenum, GLdouble const *);
      GLAPI GLvoid            glVertexStream2fATI(GLenum, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexStream2fvATI(GLenum, GLfloat const *);
      GLAPI GLvoid            glVertexStream2iATI(GLenum, GLint, GLint);
      GLAPI GLvoid            glVertexStream2ivATI(GLenum, GLint const *);
      GLAPI GLvoid            glVertexStream2sATI(GLenum, GLshort, GLshort);
      GLAPI GLvoid            glVertexStream2svATI(GLenum, GLshort const *);
      GLAPI GLvoid            glVertexStream3dATI(GLenum, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexStream3dvATI(GLenum, GLdouble const *);
      GLAPI GLvoid            glVertexStream3fATI(GLenum, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexStream3fvATI(GLenum, GLfloat const *);
      GLAPI GLvoid            glVertexStream3iATI(GLenum, GLint, GLint, GLint);
      GLAPI GLvoid            glVertexStream3ivATI(GLenum, GLint const *);
      GLAPI GLvoid            glVertexStream3sATI(GLenum, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glVertexStream3svATI(GLenum, GLshort const *);
      GLAPI GLvoid            glVertexStream4dATI(GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexStream4dvATI(GLenum, GLdouble const *);
      GLAPI GLvoid            glVertexStream4fATI(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexStream4fvATI(GLenum, GLfloat const *);
      GLAPI GLvoid            glVertexStream4iATI(GLenum, GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glVertexStream4ivATI(GLenum, GLint const *);
      GLAPI GLvoid            glVertexStream4sATI(GLenum, GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glVertexStream4svATI(GLenum, GLshort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_ATI_vertex_streams */


/* ---[ GL_EXT_422_pixels ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/422_pixels.txt */

#ifndef GL_EXT_422_pixels

   #define GL_EXT_422_pixels 1
   #define GL_EXT_422_pixels_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_422_EXT

      #define GL_422_EXT                                                  0x80cc
      #define GL_422_REV_EXT                                              0x80cd
      #define GL_422_AVERAGE_EXT                                          0x80ce
      #define GL_422_REV_AVERAGE_EXT                                      0x80cf

   #endif /* GL_422_EXT */

#endif /* GL_EXT_422_pixels */


/* ---[ GL_EXT_abgr ]---------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/abgr.txt */

#ifndef GL_EXT_abgr

   #define GL_EXT_abgr 1
   #define GL_EXT_abgr_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ABGR_EXT

      #define GL_ABGR_EXT                                                 0x8000

   #endif /* GL_ABGR_EXT */

#endif /* GL_EXT_abgr */


/* ---[ GL_EXT_bgra ]---------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/bgra.txt */

#ifndef GL_EXT_bgra

   #define GL_EXT_bgra 1
   #define GL_EXT_bgra_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_BGR_EXT

      #define GL_BGR_EXT                                                  0x80e0
      #define GL_BGRA_EXT                                                 0x80e1

   #endif /* GL_BGR_EXT */

#endif /* GL_EXT_bgra */


/* ---[ GL_EXT_blend_color ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/blend_color.txt */

#ifndef GL_EXT_blend_color

   #define GL_EXT_blend_color 1
   #define GL_EXT_blend_color_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CONSTANT_COLOR_EXT

      #define GL_CONSTANT_COLOR_EXT                                       0x8001
      #define GL_ONE_MINUS_CONSTANT_COLOR_EXT                             0x8002
      #define GL_CONSTANT_ALPHA_EXT                                       0x8003
      #define GL_ONE_MINUS_CONSTANT_ALPHA_EXT                             0x8004
      #define GL_BLEND_COLOR_EXT                                          0x8005

   #endif /* GL_CONSTANT_COLOR_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBlendColorEXT                                  OGLEXT_MAKEGLNAME(BlendColorEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBlendColorEXT(GLclampf, GLclampf, GLclampf, GLclampf);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_blend_color */


/* ---[ GL_EXT_blend_func_separate ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/blend_func_separate.txt */

#ifndef GL_EXT_blend_func_separate

   #define GL_EXT_blend_func_separate 1
   #define GL_EXT_blend_func_separate_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_BLEND_DST_RGB_EXT

      #define GL_BLEND_DST_RGB_EXT                                        0x80c8
      #define GL_BLEND_SRC_RGB_EXT                                        0x80c9
      #define GL_BLEND_DST_ALPHA_EXT                                      0x80ca
      #define GL_BLEND_SRC_ALPHA_EXT                                      0x80cb

   #endif /* GL_BLEND_DST_RGB_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBlendFuncSeparateEXT                           OGLEXT_MAKEGLNAME(BlendFuncSeparateEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBlendFuncSeparateEXT(GLenum, GLenum, GLenum, GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_blend_func_separate */


/* ---[ GL_EXT_blend_logic_op ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/blend_logic_op.txt */

#ifndef GL_EXT_blend_logic_op

   #define GL_EXT_blend_logic_op 1
   #define GL_EXT_blend_logic_op_OGLEXT 1

#endif /* GL_EXT_blend_logic_op */


/* ---[ GL_EXT_blend_minmax ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/blend_minmax.txt */

#ifndef GL_EXT_blend_minmax

   #define GL_EXT_blend_minmax 1
   #define GL_EXT_blend_minmax_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FUNC_ADD_EXT

      #define GL_FUNC_ADD_EXT                                             0x8006
      #define GL_MIN_EXT                                                  0x8007
      #define GL_MAX_EXT                                                  0x8008
      #define GL_BLEND_EQUATION_EXT                                       0x8009

   #endif /* GL_FUNC_ADD_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBlendEquationEXT                               OGLEXT_MAKEGLNAME(BlendEquationEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBlendEquationEXT(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_blend_minmax */


/* ---[ GL_EXT_blend_subtract ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/blend_subtract.txt */

#ifndef GL_EXT_blend_subtract

   #define GL_EXT_blend_subtract 1
   #define GL_EXT_blend_subtract_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FUNC_SUBTRACT_EXT

      #define GL_FUNC_SUBTRACT_EXT                                        0x800a
      #define GL_FUNC_REVERSE_SUBTRACT_EXT                                0x800b

   #endif /* GL_FUNC_SUBTRACT_EXT */

#endif /* GL_EXT_blend_subtract */


/* ---[ GL_EXT_clip_volume_hint ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/clip_volume_hint.txt */

#ifndef GL_EXT_clip_volume_hint

   #define GL_EXT_clip_volume_hint 1
   #define GL_EXT_clip_volume_hint_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CLIP_VOLUME_CLIPPING_HINT_EXT

      #define GL_CLIP_VOLUME_CLIPPING_HINT_EXT                            0x80f0

   #endif /* GL_CLIP_VOLUME_CLIPPING_HINT_EXT */

#endif /* GL_EXT_clip_volume_hint */


/* ---[ GL_EXT_cmyka ]--------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/cmyka.txt */

#ifndef GL_EXT_cmyka

   #define GL_EXT_cmyka 1
   #define GL_EXT_cmyka_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CMYK_EXT

      #define GL_CMYK_EXT                                                 0x800c
      #define GL_CMYKA_EXT                                                0x800d
      #define GL_PACK_CMYK_HINT_EXT                                       0x800e
      #define GL_UNPACK_CMYK_HINT_EXT                                     0x800f

   #endif /* GL_CMYK_EXT */

#endif /* GL_EXT_cmyka */


/* ---[ GL_EXT_color_matrix ]-------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_EXT_color_matrix

   #define GL_EXT_color_matrix 1
   #define GL_EXT_color_matrix_OGLEXT 1

#endif /* GL_EXT_color_matrix */


/* ---[ GL_EXT_color_subtable ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/color_subtable.txt */

#ifndef GL_EXT_color_subtable

   #define GL_EXT_color_subtable 1
   #define GL_EXT_color_subtable_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glColorSubTableEXT                               OGLEXT_MAKEGLNAME(ColorSubTableEXT)
      #define glCopyColorSubTableEXT                           OGLEXT_MAKEGLNAME(CopyColorSubTableEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glColorSubTableEXT(GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glCopyColorSubTableEXT(GLenum, GLsizei, GLint, GLint, GLsizei);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_color_subtable */


/* ---[ GL_EXT_compiled_vertex_array ]----------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/compiled_vertex_array.txt */

#ifndef GL_EXT_compiled_vertex_array

   #define GL_EXT_compiled_vertex_array 1
   #define GL_EXT_compiled_vertex_array_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ARRAY_ELEMENT_LOCK_FIRST_EXT

      #define GL_ARRAY_ELEMENT_LOCK_FIRST_EXT                             0x81a8
      #define GL_ARRAY_ELEMENT_LOCK_COUNT_EXT                             0x81a9

   #endif /* GL_ARRAY_ELEMENT_LOCK_FIRST_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glLockArraysEXT                                  OGLEXT_MAKEGLNAME(LockArraysEXT)
      #define glUnlockArraysEXT                                OGLEXT_MAKEGLNAME(UnlockArraysEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glLockArraysEXT(GLint, GLsizei);
      GLAPI GLvoid            glUnlockArraysEXT();

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_compiled_vertex_array */


/* ---[ GL_EXT_convolution ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/convolution.txt */

#ifndef GL_EXT_convolution

   #define GL_EXT_convolution 1
   #define GL_EXT_convolution_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CONVOLUTION_1D_EXT

      #define GL_CONVOLUTION_1D_EXT                                       0x8010
      #define GL_CONVOLUTION_2D_EXT                                       0x8011
      #define GL_SEPARABLE_2D_EXT                                         0x8012
      #define GL_CONVOLUTION_BORDER_MODE_EXT                              0x8013
      #define GL_CONVOLUTION_FILTER_SCALE_EXT                             0x8014
      #define GL_CONVOLUTION_FILTER_BIAS_EXT                              0x8015
      #define GL_REDUCE_EXT                                               0x8016
      #define GL_CONVOLUTION_FORMAT_EXT                                   0x8017
      #define GL_CONVOLUTION_WIDTH_EXT                                    0x8018
      #define GL_CONVOLUTION_HEIGHT_EXT                                   0x8019
      #define GL_MAX_CONVOLUTION_WIDTH_EXT                                0x801a
      #define GL_MAX_CONVOLUTION_HEIGHT_EXT                               0x801b
      #define GL_POST_CONVOLUTION_RED_SCALE_EXT                           0x801c
      #define GL_POST_CONVOLUTION_GREEN_SCALE_EXT                         0x801d
      #define GL_POST_CONVOLUTION_BLUE_SCALE_EXT                          0x801e
      #define GL_POST_CONVOLUTION_ALPHA_SCALE_EXT                         0x801f
      #define GL_POST_CONVOLUTION_RED_BIAS_EXT                            0x8020
      #define GL_POST_CONVOLUTION_GREEN_BIAS_EXT                          0x8021
      #define GL_POST_CONVOLUTION_BLUE_BIAS_EXT                           0x8022
      #define GL_POST_CONVOLUTION_ALPHA_BIAS_EXT                          0x8023

   #endif /* GL_CONVOLUTION_1D_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glConvolutionFilter1DEXT                         OGLEXT_MAKEGLNAME(ConvolutionFilter1DEXT)
      #define glConvolutionFilter2DEXT                         OGLEXT_MAKEGLNAME(ConvolutionFilter2DEXT)
      #define glConvolutionParameterfEXT                       OGLEXT_MAKEGLNAME(ConvolutionParameterfEXT)
      #define glConvolutionParameterfvEXT                      OGLEXT_MAKEGLNAME(ConvolutionParameterfvEXT)
      #define glConvolutionParameteriEXT                       OGLEXT_MAKEGLNAME(ConvolutionParameteriEXT)
      #define glConvolutionParameterivEXT                      OGLEXT_MAKEGLNAME(ConvolutionParameterivEXT)
      #define glCopyConvolutionFilter1DEXT                     OGLEXT_MAKEGLNAME(CopyConvolutionFilter1DEXT)
      #define glCopyConvolutionFilter2DEXT                     OGLEXT_MAKEGLNAME(CopyConvolutionFilter2DEXT)
      #define glGetConvolutionFilterEXT                        OGLEXT_MAKEGLNAME(GetConvolutionFilterEXT)
      #define glGetConvolutionParameterfvEXT                   OGLEXT_MAKEGLNAME(GetConvolutionParameterfvEXT)
      #define glGetConvolutionParameterivEXT                   OGLEXT_MAKEGLNAME(GetConvolutionParameterivEXT)
      #define glGetSeparableFilterEXT                          OGLEXT_MAKEGLNAME(GetSeparableFilterEXT)
      #define glSeparableFilter2DEXT                           OGLEXT_MAKEGLNAME(SeparableFilter2DEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glConvolutionFilter1DEXT(GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glConvolutionFilter2DEXT(GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glConvolutionParameterfEXT(GLenum, GLenum, GLfloat);
      GLAPI GLvoid            glConvolutionParameterfvEXT(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glConvolutionParameteriEXT(GLenum, GLenum, GLint);
      GLAPI GLvoid            glConvolutionParameterivEXT(GLenum, GLenum, GLint const *);
      GLAPI GLvoid            glCopyConvolutionFilter1DEXT(GLenum, GLenum, GLint, GLint, GLsizei);
      GLAPI GLvoid            glCopyConvolutionFilter2DEXT(GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
      GLAPI GLvoid            glGetConvolutionFilterEXT(GLenum, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetConvolutionParameterfvEXT(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetConvolutionParameterivEXT(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetSeparableFilterEXT(GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
      GLAPI GLvoid            glSeparableFilter2DEXT(GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_convolution */


/* ---[ GL_EXT_coordinate_frame ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/coordinate_frame.txt */

#ifndef GL_EXT_coordinate_frame

   #define GL_EXT_coordinate_frame 1
   #define GL_EXT_coordinate_frame_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TANGENT_ARRAY_EXT

      #define GL_TANGENT_ARRAY_EXT                                        0x8439
      #define GL_BINORMAL_ARRAY_EXT                                       0x843a
      #define GL_CURRENT_TANGENT_EXT                                      0x843b
      #define GL_CURRENT_BINORMAL_EXT                                     0x843c
      #define GL_TANGENT_ARRAY_TYPE_EXT                                   0x843e
      #define GL_TANGENT_ARRAY_STRIDE_EXT                                 0x843f
      #define GL_BINORMAL_ARRAY_TYPE_EXT                                  0x8440
      #define GL_BINORMAL_ARRAY_STRIDE_EXT                                0x8441
      #define GL_TANGENT_ARRAY_POINTER_EXT                                0x8442
      #define GL_BINORMAL_ARRAY_POINTER_EXT                               0x8443
      #define GL_MAP1_TANGENT_EXT                                         0x8444
      #define GL_MAP2_TANGENT_EXT                                         0x8445
      #define GL_MAP1_BINORMAL_EXT                                        0x8446
      #define GL_MAP2_BINORMAL_EXT                                        0x8447

   #endif /* GL_TANGENT_ARRAY_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBinormal3bEXT                                  OGLEXT_MAKEGLNAME(Binormal3bEXT)
      #define glBinormal3bvEXT                                 OGLEXT_MAKEGLNAME(Binormal3bvEXT)
      #define glBinormal3dEXT                                  OGLEXT_MAKEGLNAME(Binormal3dEXT)
      #define glBinormal3dvEXT                                 OGLEXT_MAKEGLNAME(Binormal3dvEXT)
      #define glBinormal3fEXT                                  OGLEXT_MAKEGLNAME(Binormal3fEXT)
      #define glBinormal3fvEXT                                 OGLEXT_MAKEGLNAME(Binormal3fvEXT)
      #define glBinormal3iEXT                                  OGLEXT_MAKEGLNAME(Binormal3iEXT)
      #define glBinormal3ivEXT                                 OGLEXT_MAKEGLNAME(Binormal3ivEXT)
      #define glBinormal3sEXT                                  OGLEXT_MAKEGLNAME(Binormal3sEXT)
      #define glBinormal3svEXT                                 OGLEXT_MAKEGLNAME(Binormal3svEXT)
      #define glBinormalPointerEXT                             OGLEXT_MAKEGLNAME(BinormalPointerEXT)
      #define glTangent3bEXT                                   OGLEXT_MAKEGLNAME(Tangent3bEXT)
      #define glTangent3bvEXT                                  OGLEXT_MAKEGLNAME(Tangent3bvEXT)
      #define glTangent3dEXT                                   OGLEXT_MAKEGLNAME(Tangent3dEXT)
      #define glTangent3dvEXT                                  OGLEXT_MAKEGLNAME(Tangent3dvEXT)
      #define glTangent3fEXT                                   OGLEXT_MAKEGLNAME(Tangent3fEXT)
      #define glTangent3fvEXT                                  OGLEXT_MAKEGLNAME(Tangent3fvEXT)
      #define glTangent3iEXT                                   OGLEXT_MAKEGLNAME(Tangent3iEXT)
      #define glTangent3ivEXT                                  OGLEXT_MAKEGLNAME(Tangent3ivEXT)
      #define glTangent3sEXT                                   OGLEXT_MAKEGLNAME(Tangent3sEXT)
      #define glTangent3svEXT                                  OGLEXT_MAKEGLNAME(Tangent3svEXT)
      #define glTangentPointerEXT                              OGLEXT_MAKEGLNAME(TangentPointerEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBinormal3bEXT(GLbyte, GLbyte, GLbyte);
      GLAPI GLvoid            glBinormal3bvEXT(GLbyte const *);
      GLAPI GLvoid            glBinormal3dEXT(GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glBinormal3dvEXT(GLdouble const *);
      GLAPI GLvoid            glBinormal3fEXT(GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glBinormal3fvEXT(GLfloat const *);
      GLAPI GLvoid            glBinormal3iEXT(GLint, GLint, GLint);
      GLAPI GLvoid            glBinormal3ivEXT(GLint const *);
      GLAPI GLvoid            glBinormal3sEXT(GLshort, GLshort, GLshort);
      GLAPI GLvoid            glBinormal3svEXT(GLshort const *);
      GLAPI GLvoid            glBinormalPointerEXT(GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glTangent3bEXT(GLbyte, GLbyte, GLbyte);
      GLAPI GLvoid            glTangent3bvEXT(GLbyte const *);
      GLAPI GLvoid            glTangent3dEXT(GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glTangent3dvEXT(GLdouble const *);
      GLAPI GLvoid            glTangent3fEXT(GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTangent3fvEXT(GLfloat const *);
      GLAPI GLvoid            glTangent3iEXT(GLint, GLint, GLint);
      GLAPI GLvoid            glTangent3ivEXT(GLint const *);
      GLAPI GLvoid            glTangent3sEXT(GLshort, GLshort, GLshort);
      GLAPI GLvoid            glTangent3svEXT(GLshort const *);
      GLAPI GLvoid            glTangentPointerEXT(GLenum, GLsizei, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_coordinate_frame */


/* ---[ GL_EXT_copy_texture ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/copy_texture.txt */

#ifndef GL_EXT_copy_texture

   #define GL_EXT_copy_texture 1
   #define GL_EXT_copy_texture_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glCopyTexImage1DEXT                              OGLEXT_MAKEGLNAME(CopyTexImage1DEXT)
      #define glCopyTexImage2DEXT                              OGLEXT_MAKEGLNAME(CopyTexImage2DEXT)
      #define glCopyTexSubImage1DEXT                           OGLEXT_MAKEGLNAME(CopyTexSubImage1DEXT)
      #define glCopyTexSubImage2DEXT                           OGLEXT_MAKEGLNAME(CopyTexSubImage2DEXT)
      #define glCopyTexSubImage3DEXT                           OGLEXT_MAKEGLNAME(CopyTexSubImage3DEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glCopyTexImage1DEXT(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
      GLAPI GLvoid            glCopyTexImage2DEXT(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
      GLAPI GLvoid            glCopyTexSubImage1DEXT(GLenum, GLint, GLint, GLint, GLint, GLsizei);
      GLAPI GLvoid            glCopyTexSubImage2DEXT(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
      GLAPI GLvoid            glCopyTexSubImage3DEXT(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_copy_texture */


/* ---[ GL_EXT_cull_vertex ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/cull_vertex.txt */

#ifndef GL_EXT_cull_vertex

   #define GL_EXT_cull_vertex 1
   #define GL_EXT_cull_vertex_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CULL_VERTEX_EXT

      #define GL_CULL_VERTEX_EXT                                          0x81aa
      #define GL_CULL_VERTEX_EYE_POSITION_EXT                             0x81ab
      #define GL_CULL_VERTEX_OBJECT_POSITION_EXT                          0x81ac

   #endif /* GL_CULL_VERTEX_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glCullParameterdvEXT                             OGLEXT_MAKEGLNAME(CullParameterdvEXT)
      #define glCullParameterfvEXT                             OGLEXT_MAKEGLNAME(CullParameterfvEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glCullParameterdvEXT(GLenum, GLdouble *);
      GLAPI GLvoid            glCullParameterfvEXT(GLenum, GLfloat *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_cull_vertex */


/* ---[ GL_EXT_depth_bounds_test ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/depth_bounds_test.txt */

#ifndef GL_EXT_depth_bounds_test

   #define GL_EXT_depth_bounds_test 1
   #define GL_EXT_depth_bounds_test_OGLEXT 1

#endif /* GL_EXT_depth_bounds_test */


/* ---[ GL_EXT_draw_range_elements ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/draw_range_elements.txt */

#ifndef GL_EXT_draw_range_elements

   #define GL_EXT_draw_range_elements 1
   #define GL_EXT_draw_range_elements_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MAX_ELEMENTS_VERTICES_EXT

      #define GL_MAX_ELEMENTS_VERTICES_EXT                                0x80e8
      #define GL_MAX_ELEMENTS_INDICES_EXT                                 0x80e9

   #endif /* GL_MAX_ELEMENTS_VERTICES_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDrawRangeElementsEXT                           OGLEXT_MAKEGLNAME(DrawRangeElementsEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDrawRangeElementsEXT(GLenum, GLuint, GLuint, GLsizei, GLenum, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_draw_range_elements */


/* ---[ GL_EXT_fog_coord ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/fog_coord.txt */

#ifndef GL_EXT_fog_coord

   #define GL_EXT_fog_coord 1
   #define GL_EXT_fog_coord_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FOG_COORDINATE_SOURCE_EXT

      #define GL_FOG_COORDINATE_SOURCE_EXT                                0x8450
      #define GL_FOG_COORDINATE_EXT                                       0x8451
      #define GL_FRAGMENT_DEPTH_EXT                                       0x8452
      #define GL_CURRENT_FOG_COORDINATE_EXT                               0x8453
      #define GL_FOG_COORDINATE_ARRAY_TYPE_EXT                            0x8454
      #define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT                          0x8455
      #define GL_FOG_COORDINATE_ARRAY_POINTER_EXT                         0x8456
      #define GL_FOG_COORDINATE_ARRAY_EXT                                 0x8457

   #endif /* GL_FOG_COORDINATE_SOURCE_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFogCoorddEXT                                   OGLEXT_MAKEGLNAME(FogCoorddEXT)
      #define glFogCoorddvEXT                                  OGLEXT_MAKEGLNAME(FogCoorddvEXT)
      #define glFogCoordfEXT                                   OGLEXT_MAKEGLNAME(FogCoordfEXT)
      #define glFogCoordfvEXT                                  OGLEXT_MAKEGLNAME(FogCoordfvEXT)
      #define glFogCoordPointerEXT                             OGLEXT_MAKEGLNAME(FogCoordPointerEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFogCoorddEXT(GLdouble);
      GLAPI GLvoid            glFogCoorddvEXT(GLdouble const *);
      GLAPI GLvoid            glFogCoordfEXT(GLfloat);
      GLAPI GLvoid            glFogCoordfvEXT(GLfloat const *);
      GLAPI GLvoid            glFogCoordPointerEXT(GLenum, GLsizei, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_fog_coord */


/* ---[ GL_EXT_fragment_lighting ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/fragment_lighting.txt */

#ifndef GL_EXT_fragment_lighting

   #define GL_EXT_fragment_lighting 1
   #define GL_EXT_fragment_lighting_OGLEXT 1

#endif /* GL_EXT_fragment_lighting */


/* ---[ GL_EXT_histogram ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/histogram.txt */

#ifndef GL_EXT_histogram

   #define GL_EXT_histogram 1
   #define GL_EXT_histogram_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_HISTOGRAM_EXT

      #define GL_HISTOGRAM_EXT                                            0x8024
      #define GL_PROXY_HISTOGRAM_EXT                                      0x8025
      #define GL_HISTOGRAM_WIDTH_EXT                                      0x8026
      #define GL_HISTOGRAM_FORMAT_EXT                                     0x8027
      #define GL_HISTOGRAM_RED_SIZE_EXT                                   0x8028
      #define GL_HISTOGRAM_GREEN_SIZE_EXT                                 0x8029
      #define GL_HISTOGRAM_BLUE_SIZE_EXT                                  0x802a
      #define GL_HISTOGRAM_ALPHA_SIZE_EXT                                 0x802b
      #define GL_HISTOGRAM_LUMINANCE_SIZE_EXT                             0x802c
      #define GL_HISTOGRAM_SINK_EXT                                       0x802d
      #define GL_MINMAX_EXT                                               0x802e
      #define GL_MINMAX_FORMAT_EXT                                        0x802f
      #define GL_MINMAX_SINK_EXT                                          0x8030
      #define GL_TABLE_TOO_LARGE_EXT                                      0x8031

   #endif /* GL_HISTOGRAM_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetHistogramEXT                                OGLEXT_MAKEGLNAME(GetHistogramEXT)
      #define glGetHistogramParameterfvEXT                     OGLEXT_MAKEGLNAME(GetHistogramParameterfvEXT)
      #define glGetHistogramParameterivEXT                     OGLEXT_MAKEGLNAME(GetHistogramParameterivEXT)
      #define glGetMinmaxEXT                                   OGLEXT_MAKEGLNAME(GetMinmaxEXT)
      #define glGetMinmaxParameterfvEXT                        OGLEXT_MAKEGLNAME(GetMinmaxParameterfvEXT)
      #define glGetMinmaxParameterivEXT                        OGLEXT_MAKEGLNAME(GetMinmaxParameterivEXT)
      #define glHistogramEXT                                   OGLEXT_MAKEGLNAME(HistogramEXT)
      #define glMinmaxEXT                                      OGLEXT_MAKEGLNAME(MinmaxEXT)
      #define glResetHistogramEXT                              OGLEXT_MAKEGLNAME(ResetHistogramEXT)
      #define glResetMinmaxEXT                                 OGLEXT_MAKEGLNAME(ResetMinmaxEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetHistogramEXT(GLenum, GLboolean, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetHistogramParameterfvEXT(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetHistogramParameterivEXT(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetMinmaxEXT(GLenum, GLboolean, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetMinmaxParameterfvEXT(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetMinmaxParameterivEXT(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glHistogramEXT(GLenum, GLsizei, GLenum, GLboolean);
      GLAPI GLvoid            glMinmaxEXT(GLenum, GLenum, GLboolean);
      GLAPI GLvoid            glResetHistogramEXT(GLenum);
      GLAPI GLvoid            glResetMinmaxEXT(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_histogram */


/* ---[ GL_EXT_index_array_formats ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/index_array_formats.txt */

#ifndef GL_EXT_index_array_formats

   #define GL_EXT_index_array_formats 1
   #define GL_EXT_index_array_formats_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_IUI_V2F_EXT

      #define GL_IUI_V2F_EXT                                              0x81ad
      #define GL_IUI_V3F_EXT                                              0x81ae
      #define GL_IUI_N3F_V2F_EXT                                          0x81af
      #define GL_IUI_N3F_V3F_EXT                                          0x81b0
      #define GL_T2F_IUI_V2F_EXT                                          0x81b1
      #define GL_T2F_IUI_V3F_EXT                                          0x81b2
      #define GL_T2F_IUI_N3F_V2F_EXT                                      0x81b3
      #define GL_T2F_IUI_N3F_V3F_EXT                                      0x81b4

   #endif /* GL_IUI_V2F_EXT */

#endif /* GL_EXT_index_array_formats */


/* ---[ GL_EXT_index_func ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/index_func.txt */

#ifndef GL_EXT_index_func

   #define GL_EXT_index_func 1
   #define GL_EXT_index_func_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_INDEX_TEST_EXT

      #define GL_INDEX_TEST_EXT                                           0x81b5
      #define GL_INDEX_TEST_FUNC_EXT                                      0x81b6
      #define GL_INDEX_TEST_REF_EXT                                       0x81b7

   #endif /* GL_INDEX_TEST_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glIndexFuncEXT                                   OGLEXT_MAKEGLNAME(IndexFuncEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glIndexFuncEXT(GLenum, GLclampf);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_index_func */


/* ---[ GL_EXT_index_material ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/index_material.txt */

#ifndef GL_EXT_index_material

   #define GL_EXT_index_material 1
   #define GL_EXT_index_material_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_INDEX_MATERIAL_EXT

      #define GL_INDEX_MATERIAL_EXT                                       0x81b8
      #define GL_INDEX_MATERIAL_PARAMETER_EXT                             0x81b9
      #define GL_INDEX_MATERIAL_FACE_EXT                                  0x81ba

   #endif /* GL_INDEX_MATERIAL_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glIndexMaterialEXT                               OGLEXT_MAKEGLNAME(IndexMaterialEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glIndexMaterialEXT(GLenum, GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_index_material */


/* ---[ GL_EXT_index_texture ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/index_texture.txt */

#ifndef GL_EXT_index_texture

   #define GL_EXT_index_texture 1
   #define GL_EXT_index_texture_OGLEXT 1

#endif /* GL_EXT_index_texture */


/* ---[ GL_EXT_light_texture ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/light_texture.txt */

#ifndef GL_EXT_light_texture

   #define GL_EXT_light_texture 1
   #define GL_EXT_light_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FRAGMENT_MATERIAL_EXT

      #define GL_FRAGMENT_MATERIAL_EXT                                    0x8349
      #define GL_FRAGMENT_NORMAL_EXT                                      0x834a
      #define GL_FRAGMENT_COLOR_EXT                                       0x834c
      #define GL_ATTENUATION_EXT                                          0x834d
      #define GL_SHADOW_ATTENUATION_EXT                                   0x834e
      #define GL_TEXTURE_APPLICATION_MODE_EXT                             0x834f
      #define GL_TEXTURE_LIGHT_EXT                                        0x8350
      #define GL_TEXTURE_MATERIAL_FACE_EXT                                0x8351
      #define GL_TEXTURE_MATERIAL_PARAMETER_EXT                           0x8352

   #endif /* GL_FRAGMENT_MATERIAL_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glApplyTextureEXT                                OGLEXT_MAKEGLNAME(ApplyTextureEXT)
      #define glTextureLightEXT                                OGLEXT_MAKEGLNAME(TextureLightEXT)
      #define glTextureMaterialEXT                             OGLEXT_MAKEGLNAME(TextureMaterialEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glApplyTextureEXT(GLenum);
      GLAPI GLvoid            glTextureLightEXT(GLenum);
      GLAPI GLvoid            glTextureMaterialEXT(GLenum, GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_light_texture */


/* ---[ GL_EXT_misc_attribute ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/misc_attribute.txt */

#ifndef GL_EXT_misc_attribute

   #define GL_EXT_misc_attribute 1
   #define GL_EXT_misc_attribute_OGLEXT 1

#endif /* GL_EXT_misc_attribute */


/* ---[ GL_EXT_multisample ]--------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_EXT_multisample

   #define GL_EXT_multisample 1
   #define GL_EXT_multisample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MULTISAMPLE_EXT

      #define GL_MULTISAMPLE_EXT                                          0x809d
      #define GL_SAMPLE_ALPHA_TO_MASK_EXT                                 0x809e
      #define GL_SAMPLE_ALPHA_TO_ONE_EXT                                  0x809f
      #define GL_SAMPLE_MASK_EXT                                          0x80a0
      #define GL_1PASS_EXT                                                0x80a1
      #define GL_2PASS_0_EXT                                              0x80a2
      #define GL_2PASS_1_EXT                                              0x80a3
      #define GL_4PASS_0_EXT                                              0x80a4
      #define GL_4PASS_1_EXT                                              0x80a5
      #define GL_4PASS_2_EXT                                              0x80a6
      #define GL_4PASS_3_EXT                                              0x80a7
      #define GL_SAMPLE_BUFFERS_EXT                                       0x80a8
      #define GL_SAMPLES_EXT                                              0x80a9
      #define GL_SAMPLE_MASK_VALUE_EXT                                    0x80aa
      #define GL_SAMPLE_MASK_INVERT_EXT                                   0x80ab
      #define GL_SAMPLE_PATTERN_EXT                                       0x80ac
      #define GL_MULTISAMPLE_BIT_EXT                                      0x20000000

   #endif /* GL_MULTISAMPLE_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glSampleMaskEXT                                  OGLEXT_MAKEGLNAME(SampleMaskEXT)
      #define glSamplePatternEXT                               OGLEXT_MAKEGLNAME(SamplePatternEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glSampleMaskEXT(GLclampf, GLboolean);
      GLAPI GLvoid            glSamplePatternEXT(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_multisample */


/* ---[ GL_EXT_multitexture ]-------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_EXT_multitexture

   #define GL_EXT_multitexture 1
   #define GL_EXT_multitexture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SELECTED_TEXTURE_EXT

      #define GL_SELECTED_TEXTURE_EXT                                     0x83c0
      #define GL_SELECTED_TEXTURE_COORD_SET_EXT                           0x83c1
      #define GL_SELECTED_TEXTURE_TRANSFORM_EXT                           0x83c2
      #define GL_MAX_TEXTURES_EXT                                         0x83c3
      #define GL_MAX_TEXTURE_COORD_SETS_EXT                               0x83c4
      #define GL_TEXTURE_ENV_COORD_SET_EXT                                0x83c5
      #define GL_TEXTURE0_EXT                                             0x83c6
      #define GL_TEXTURE1_EXT                                             0x83c7
      #define GL_TEXTURE2_EXT                                             0x83c8
      #define GL_TEXTURE3_EXT                                             0x83c9

   #endif /* GL_SELECTED_TEXTURE_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glInterleavedTextureCoordSetsEXT                 OGLEXT_MAKEGLNAME(InterleavedTextureCoordSetsEXT)
      #define glMultiTexCoord1dEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord1dEXT)
      #define glMultiTexCoord1dvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord1dvEXT)
      #define glMultiTexCoord1fEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord1fEXT)
      #define glMultiTexCoord1fvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord1fvEXT)
      #define glMultiTexCoord1iEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord1iEXT)
      #define glMultiTexCoord1ivEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord1ivEXT)
      #define glMultiTexCoord1sEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord1sEXT)
      #define glMultiTexCoord1svEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord1svEXT)
      #define glMultiTexCoord2dEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord2dEXT)
      #define glMultiTexCoord2dvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord2dvEXT)
      #define glMultiTexCoord2fEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord2fEXT)
      #define glMultiTexCoord2fvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord2fvEXT)
      #define glMultiTexCoord2iEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord2iEXT)
      #define glMultiTexCoord2ivEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord2ivEXT)
      #define glMultiTexCoord2sEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord2sEXT)
      #define glMultiTexCoord2svEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord2svEXT)
      #define glMultiTexCoord3dEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord3dEXT)
      #define glMultiTexCoord3dvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord3dvEXT)
      #define glMultiTexCoord3fEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord3fEXT)
      #define glMultiTexCoord3fvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord3fvEXT)
      #define glMultiTexCoord3iEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord3iEXT)
      #define glMultiTexCoord3ivEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord3ivEXT)
      #define glMultiTexCoord3sEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord3sEXT)
      #define glMultiTexCoord3svEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord3svEXT)
      #define glMultiTexCoord4dEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord4dEXT)
      #define glMultiTexCoord4dvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord4dvEXT)
      #define glMultiTexCoord4fEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord4fEXT)
      #define glMultiTexCoord4fvEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord4fvEXT)
      #define glMultiTexCoord4iEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord4iEXT)
      #define glMultiTexCoord4ivEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord4ivEXT)
      #define glMultiTexCoord4sEXT                             OGLEXT_MAKEGLNAME(MultiTexCoord4sEXT)
      #define glMultiTexCoord4svEXT                            OGLEXT_MAKEGLNAME(MultiTexCoord4svEXT)
      #define glSelectTextureCoordSetEXT                       OGLEXT_MAKEGLNAME(SelectTextureCoordSetEXT)
      #define glSelectTextureEXT                               OGLEXT_MAKEGLNAME(SelectTextureEXT)
      #define glSelectTextureTransformEXT                      OGLEXT_MAKEGLNAME(SelectTextureTransformEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glInterleavedTextureCoordSetsEXT(GLint);
      GLAPI GLvoid            glMultiTexCoord1dEXT(GLenum, GLdouble);
      GLAPI GLvoid            glMultiTexCoord1dvEXT(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord1fEXT(GLenum, GLfloat);
      GLAPI GLvoid            glMultiTexCoord1fvEXT(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord1iEXT(GLenum, GLint);
      GLAPI GLvoid            glMultiTexCoord1ivEXT(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord1sEXT(GLenum, GLshort);
      GLAPI GLvoid            glMultiTexCoord1svEXT(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord2dEXT(GLenum, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord2dvEXT(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord2fEXT(GLenum, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord2fvEXT(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord2iEXT(GLenum, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord2ivEXT(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord2sEXT(GLenum, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord2svEXT(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord3dEXT(GLenum, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord3dvEXT(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord3fEXT(GLenum, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord3fvEXT(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord3iEXT(GLenum, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord3ivEXT(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord3sEXT(GLenum, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord3svEXT(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord4dEXT(GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord4dvEXT(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord4fEXT(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord4fvEXT(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord4iEXT(GLenum, GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord4ivEXT(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord4sEXT(GLenum, GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord4svEXT(GLenum, GLshort const *);
      GLAPI GLvoid            glSelectTextureCoordSetEXT(GLenum);
      GLAPI GLvoid            glSelectTextureEXT(GLenum);
      GLAPI GLvoid            glSelectTextureTransformEXT(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_multitexture */


/* ---[ GL_EXT_multi_draw_arrays ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/multi_draw_arrays.txt */

#ifndef GL_EXT_multi_draw_arrays

   #define GL_EXT_multi_draw_arrays 1
   #define GL_EXT_multi_draw_arrays_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glMultiDrawArraysEXT                             OGLEXT_MAKEGLNAME(MultiDrawArraysEXT)
      #define glMultiDrawElementsEXT                           OGLEXT_MAKEGLNAME(MultiDrawElementsEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glMultiDrawArraysEXT(GLenum, GLint *, GLsizei *, GLsizei);
      GLAPI GLvoid            glMultiDrawElementsEXT(GLenum, GLsizei const *, GLenum, GLvoid const * *, GLsizei);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_multi_draw_arrays */


/* ---[ GL_EXT_packed_pixels ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/packed_pixels.txt */

#ifndef GL_EXT_packed_pixels

   #define GL_EXT_packed_pixels 1
   #define GL_EXT_packed_pixels_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_UNSIGNED_BYTE_3_3_2_EXT

      #define GL_UNSIGNED_BYTE_3_3_2_EXT                                  0x8032
      #define GL_UNSIGNED_SHORT_4_4_4_4_EXT                               0x8033
      #define GL_UNSIGNED_SHORT_5_5_5_1_EXT                               0x8034
      #define GL_UNSIGNED_INT_8_8_8_8_EXT                                 0x8035
      #define GL_UNSIGNED_INT_10_10_10_2_EXT                              0x8036

   #endif /* GL_UNSIGNED_BYTE_3_3_2_EXT */

#endif /* GL_EXT_packed_pixels */


/* ---[ GL_EXT_paletted_texture ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/paletted_texture.txt */

#ifndef GL_EXT_paletted_texture

   #define GL_EXT_paletted_texture 1
   #define GL_EXT_paletted_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COLOR_INDEX1_EXT

      #define GL_COLOR_INDEX1_EXT                                         0x80e2
      #define GL_COLOR_INDEX2_EXT                                         0x80e3
      #define GL_COLOR_INDEX4_EXT                                         0x80e4
      #define GL_COLOR_INDEX8_EXT                                         0x80e5
      #define GL_COLOR_INDEX12_EXT                                        0x80e6
      #define GL_COLOR_INDEX16_EXT                                        0x80e7
      #define GL_TEXTURE_INDEX_SIZE_EXT                                   0x80ed

   #endif /* GL_COLOR_INDEX1_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glColorTableEXT                                  OGLEXT_MAKEGLNAME(ColorTableEXT)
      #define glGetColorTableEXT                               OGLEXT_MAKEGLNAME(GetColorTableEXT)
      #define glGetColorTableParameterfvEXT                    OGLEXT_MAKEGLNAME(GetColorTableParameterfvEXT)
      #define glGetColorTableParameterivEXT                    OGLEXT_MAKEGLNAME(GetColorTableParameterivEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glColorTableEXT(GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glGetColorTableEXT(GLenum, GLenum, GLenum, GLvoid *);
      GLAPI GLvoid            glGetColorTableParameterfvEXT(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetColorTableParameterivEXT(GLenum, GLenum, GLint *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_paletted_texture */


/* ---[ GL_EXT_pixel_transform ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/pixel_transform.txt */

#ifndef GL_EXT_pixel_transform

   #define GL_EXT_pixel_transform 1
   #define GL_EXT_pixel_transform_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PIXEL_TRANSFORM_2D_EXT

      #define GL_PIXEL_TRANSFORM_2D_EXT                                   0x8330
      #define GL_PIXEL_MAG_FILTER_EXT                                     0x8331
      #define GL_PIXEL_MIN_FILTER_EXT                                     0x8332
      #define GL_PIXEL_CUBIC_WEIGHT_EXT                                   0x8333
      #define GL_CUBIC_EXT                                                0x8334
      #define GL_AVERAGE_EXT                                              0x8335
      #define GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT                       0x8336
      #define GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT                   0x8337
      #define GL_PIXEL_TRANSFORM_2D_MATRIX_EXT                            0x8338

   #endif /* GL_PIXEL_TRANSFORM_2D_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPixelTransformParameterfEXT                    OGLEXT_MAKEGLNAME(PixelTransformParameterfEXT)
      #define glPixelTransformParameterfvEXT                   OGLEXT_MAKEGLNAME(PixelTransformParameterfvEXT)
      #define glPixelTransformParameteriEXT                    OGLEXT_MAKEGLNAME(PixelTransformParameteriEXT)
      #define glPixelTransformParameterivEXT                   OGLEXT_MAKEGLNAME(PixelTransformParameterivEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPixelTransformParameterfEXT(GLenum, GLenum, GLfloat);
      GLAPI GLvoid            glPixelTransformParameterfvEXT(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glPixelTransformParameteriEXT(GLenum, GLenum, GLint);
      GLAPI GLvoid            glPixelTransformParameterivEXT(GLenum, GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_pixel_transform */


/* ---[ GL_EXT_pixel_transform_color_table ]----------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/pixel_transform_color_table.txt */

#ifndef GL_EXT_pixel_transform_color_table

   #define GL_EXT_pixel_transform_color_table 1
   #define GL_EXT_pixel_transform_color_table_OGLEXT 1

#endif /* GL_EXT_pixel_transform_color_table */


/* ---[ GL_EXT_point_parameters ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/point_parameters.txt */

#ifndef GL_EXT_point_parameters

   #define GL_EXT_point_parameters 1
   #define GL_EXT_point_parameters_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_POINT_SIZE_MIN_EXT

      #define GL_POINT_SIZE_MIN_EXT                                       0x8126
      #define GL_POINT_SIZE_MAX_EXT                                       0x8127
      #define GL_POINT_FADE_THRESHOLD_SIZE_EXT                            0x8128
      #define GL_DISTANCE_ATTENUATION_EXT                                 0x8129

   #endif /* GL_POINT_SIZE_MIN_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPointParameterfEXT                             OGLEXT_MAKEGLNAME(PointParameterfEXT)
      #define glPointParameterfvEXT                            OGLEXT_MAKEGLNAME(PointParameterfvEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPointParameterfEXT(GLenum, GLfloat);
      GLAPI GLvoid            glPointParameterfvEXT(GLenum, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_point_parameters */


/* ---[ GL_EXT_polygon_offset ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/polygon_offset.txt */

#ifndef GL_EXT_polygon_offset

   #define GL_EXT_polygon_offset 1
   #define GL_EXT_polygon_offset_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_POLYGON_OFFSET_EXT

      #define GL_POLYGON_OFFSET_EXT                                       0x8037
      #define GL_POLYGON_OFFSET_FACTOR_EXT                                0x8038
      #define GL_POLYGON_OFFSET_BIAS_EXT                                  0x8039

   #endif /* GL_POLYGON_OFFSET_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPolygonOffsetEXT                               OGLEXT_MAKEGLNAME(PolygonOffsetEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPolygonOffsetEXT(GLfloat, GLfloat);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_polygon_offset */


/* ---[ GL_EXT_rescale_normal ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/rescale_normal.txt */

#ifndef GL_EXT_rescale_normal

   #define GL_EXT_rescale_normal 1
   #define GL_EXT_rescale_normal_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_RESCALE_NORMAL_EXT

      #define GL_RESCALE_NORMAL_EXT                                       0x803a

   #endif /* GL_RESCALE_NORMAL_EXT */

#endif /* GL_EXT_rescale_normal */


/* ---[ GL_EXT_secondary_color ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/secondary_color.txt */

#ifndef GL_EXT_secondary_color

   #define GL_EXT_secondary_color 1
   #define GL_EXT_secondary_color_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COLOR_SUM_EXT

      #define GL_COLOR_SUM_EXT                                            0x8458
      #define GL_CURRENT_SECONDARY_COLOR_EXT                              0x8459
      #define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT                           0x845a
      #define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT                           0x845b
      #define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT                         0x845c
      #define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT                        0x845d
      #define GL_SECONDARY_COLOR_ARRAY_EXT                                0x845e

   #endif /* GL_COLOR_SUM_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glSecondaryColor3bEXT                            OGLEXT_MAKEGLNAME(SecondaryColor3bEXT)
      #define glSecondaryColor3bvEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3bvEXT)
      #define glSecondaryColor3dEXT                            OGLEXT_MAKEGLNAME(SecondaryColor3dEXT)
      #define glSecondaryColor3dvEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3dvEXT)
      #define glSecondaryColor3fEXT                            OGLEXT_MAKEGLNAME(SecondaryColor3fEXT)
      #define glSecondaryColor3fvEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3fvEXT)
      #define glSecondaryColor3iEXT                            OGLEXT_MAKEGLNAME(SecondaryColor3iEXT)
      #define glSecondaryColor3ivEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3ivEXT)
      #define glSecondaryColor3sEXT                            OGLEXT_MAKEGLNAME(SecondaryColor3sEXT)
      #define glSecondaryColor3svEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3svEXT)
      #define glSecondaryColor3ubEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3ubEXT)
      #define glSecondaryColor3ubvEXT                          OGLEXT_MAKEGLNAME(SecondaryColor3ubvEXT)
      #define glSecondaryColor3uiEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3uiEXT)
      #define glSecondaryColor3uivEXT                          OGLEXT_MAKEGLNAME(SecondaryColor3uivEXT)
      #define glSecondaryColor3usEXT                           OGLEXT_MAKEGLNAME(SecondaryColor3usEXT)
      #define glSecondaryColor3usvEXT                          OGLEXT_MAKEGLNAME(SecondaryColor3usvEXT)
      #define glSecondaryColorPointerEXT                       OGLEXT_MAKEGLNAME(SecondaryColorPointerEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glSecondaryColor3bEXT(GLbyte, GLbyte, GLbyte);
      GLAPI GLvoid            glSecondaryColor3bvEXT(GLbyte const *);
      GLAPI GLvoid            glSecondaryColor3dEXT(GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glSecondaryColor3dvEXT(GLdouble const *);
      GLAPI GLvoid            glSecondaryColor3fEXT(GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glSecondaryColor3fvEXT(GLfloat const *);
      GLAPI GLvoid            glSecondaryColor3iEXT(GLint, GLint, GLint);
      GLAPI GLvoid            glSecondaryColor3ivEXT(GLint const *);
      GLAPI GLvoid            glSecondaryColor3sEXT(GLshort, GLshort, GLshort);
      GLAPI GLvoid            glSecondaryColor3svEXT(GLshort const *);
      GLAPI GLvoid            glSecondaryColor3ubEXT(GLubyte, GLubyte, GLubyte);
      GLAPI GLvoid            glSecondaryColor3ubvEXT(GLubyte const *);
      GLAPI GLvoid            glSecondaryColor3uiEXT(GLuint, GLuint, GLuint);
      GLAPI GLvoid            glSecondaryColor3uivEXT(GLuint const *);
      GLAPI GLvoid            glSecondaryColor3usEXT(GLushort, GLushort, GLushort);
      GLAPI GLvoid            glSecondaryColor3usvEXT(GLushort const *);
      GLAPI GLvoid            glSecondaryColorPointerEXT(GLint, GLenum, GLsizei, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_secondary_color */


/* ---[ GL_EXT_separate_specular_color ]--------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/separate_specular_color.txt */

#ifndef GL_EXT_separate_specular_color

   #define GL_EXT_separate_specular_color 1
   #define GL_EXT_separate_specular_color_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_LIGHT_MODEL_COLOR_CONTROL_EXT

      #define GL_LIGHT_MODEL_COLOR_CONTROL_EXT                            0x81f8
      #define GL_SINGLE_COLOR_EXT                                         0x81f9
      #define GL_SEPARATE_SPECULAR_COLOR_EXT                              0x81fa

   #endif /* GL_LIGHT_MODEL_COLOR_CONTROL_EXT */

#endif /* GL_EXT_separate_specular_color */


/* ---[ GL_EXT_shadow_funcs ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/shadow_funcs.txt */

#ifndef GL_EXT_shadow_funcs

   #define GL_EXT_shadow_funcs 1
   #define GL_EXT_shadow_funcs_OGLEXT 1

#endif /* GL_EXT_shadow_funcs */


/* ---[ GL_EXT_shared_texture_palette ]---------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/shared_texture_palette.txt */

#ifndef GL_EXT_shared_texture_palette

   #define GL_EXT_shared_texture_palette 1
   #define GL_EXT_shared_texture_palette_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SHARED_TEXTURE_PALETTE_EXT

      #define GL_SHARED_TEXTURE_PALETTE_EXT                               0x81fb

   #endif /* GL_SHARED_TEXTURE_PALETTE_EXT */

#endif /* GL_EXT_shared_texture_palette */


/* ---[ GL_EXT_stencil_two_side ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/stencil_two_side.txt */

#ifndef GL_EXT_stencil_two_side

   #define GL_EXT_stencil_two_side 1
   #define GL_EXT_stencil_two_side_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_STENCIL_TEST_TWO_SIDE_EXT

      #define GL_STENCIL_TEST_TWO_SIDE_EXT                                0x8910
      #define GL_ACTIVE_STENCIL_FACE_EXT                                  0x8911

   #endif /* GL_STENCIL_TEST_TWO_SIDE_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glActiveStencilFaceEXT                           OGLEXT_MAKEGLNAME(ActiveStencilFaceEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glActiveStencilFaceEXT(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_stencil_two_side */


/* ---[ GL_EXT_stencil_wrap ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/stencil_wrap.txt */

#ifndef GL_EXT_stencil_wrap

   #define GL_EXT_stencil_wrap 1
   #define GL_EXT_stencil_wrap_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_INCR_WRAP_EXT

      #define GL_INCR_WRAP_EXT                                            0x8507
      #define GL_DECR_WRAP_EXT                                            0x8508

   #endif /* GL_INCR_WRAP_EXT */

#endif /* GL_EXT_stencil_wrap */


/* ---[ GL_EXT_subtexture ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/subtexture.txt */

#ifndef GL_EXT_subtexture

   #define GL_EXT_subtexture 1
   #define GL_EXT_subtexture_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glTexSubImage1DEXT                               OGLEXT_MAKEGLNAME(TexSubImage1DEXT)
      #define glTexSubImage2DEXT                               OGLEXT_MAKEGLNAME(TexSubImage2DEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glTexSubImage1DEXT(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glTexSubImage2DEXT(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_subtexture */


/* ---[ GL_EXT_texture ]------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture.txt */

#ifndef GL_EXT_texture

   #define GL_EXT_texture 1
   #define GL_EXT_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ALPHA4_EXT

      #define GL_ALPHA4_EXT                                               0x803b
      #define GL_ALPHA8_EXT                                               0x803c
      #define GL_ALPHA12_EXT                                              0x803d
      #define GL_ALPHA16_EXT                                              0x803e
      #define GL_LUMINANCE4_EXT                                           0x803f
      #define GL_LUMINANCE8_EXT                                           0x8040
      #define GL_LUMINANCE12_EXT                                          0x8041
      #define GL_LUMINANCE16_EXT                                          0x8042
      #define GL_LUMINANCE4_ALPHA4_EXT                                    0x8043
      #define GL_LUMINANCE6_ALPHA2_EXT                                    0x8044
      #define GL_LUMINANCE8_ALPHA8_EXT                                    0x8045
      #define GL_LUMINANCE12_ALPHA4_EXT                                   0x8046
      #define GL_LUMINANCE12_ALPHA12_EXT                                  0x8047
      #define GL_LUMINANCE16_ALPHA16_EXT                                  0x8048
      #define GL_INTENSITY_EXT                                            0x8049
      #define GL_INTENSITY4_EXT                                           0x804a
      #define GL_INTENSITY8_EXT                                           0x804b
      #define GL_INTENSITY12_EXT                                          0x804c
      #define GL_INTENSITY16_EXT                                          0x804d
      #define GL_RGB2_EXT                                                 0x804e
      #define GL_RGB4_EXT                                                 0x804f
      #define GL_RGB5_EXT                                                 0x8050
      #define GL_RGB8_EXT                                                 0x8051
      #define GL_RGB10_EXT                                                0x8052
      #define GL_RGB12_EXT                                                0x8053
      #define GL_RGB16_EXT                                                0x8054
      #define GL_RGBA2_EXT                                                0x8055
      #define GL_RGBA4_EXT                                                0x8056
      #define GL_RGB5_A1_EXT                                              0x8057
      #define GL_RGBA8_EXT                                                0x8058
      #define GL_RGB10_A2_EXT                                             0x8059
      #define GL_RGBA12_EXT                                               0x805a
      #define GL_RGBA16_EXT                                               0x805b
      #define GL_TEXTURE_RED_SIZE_EXT                                     0x805c
      #define GL_TEXTURE_GREEN_SIZE_EXT                                   0x805d
      #define GL_TEXTURE_BLUE_SIZE_EXT                                    0x805e
      #define GL_TEXTURE_ALPHA_SIZE_EXT                                   0x805f
      #define GL_TEXTURE_LUMINANCE_SIZE_EXT                               0x8060
      #define GL_TEXTURE_INTENSITY_SIZE_EXT                               0x8061
      #define GL_REPLACE_EXT                                              0x8062
      #define GL_PROXY_TEXTURE_1D_EXT                                     0x8063
      #define GL_PROXY_TEXTURE_2D_EXT                                     0x8064
      #define GL_TEXTURE_TOO_LARGE_EXT                                    0x8065

   #endif /* GL_ALPHA4_EXT */

#endif /* GL_EXT_texture */


/* ---[ GL_EXT_texture3D ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture3D.txt */

#ifndef GL_EXT_texture3D

   #define GL_EXT_texture3D 1
   #define GL_EXT_texture3D_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PACK_SKIP_IMAGES_EXT

      #define GL_PACK_SKIP_IMAGES_EXT                                     0x806b
      #define GL_PACK_IMAGE_HEIGHT_EXT                                    0x806c
      #define GL_UNPACK_SKIP_IMAGES_EXT                                   0x806d
      #define GL_UNPACK_IMAGE_HEIGHT_EXT                                  0x806e
      #define GL_TEXTURE_3D_EXT                                           0x806f
      #define GL_PROXY_TEXTURE_3D_EXT                                     0x8070
      #define GL_TEXTURE_DEPTH_EXT                                        0x8071
      #define GL_TEXTURE_WRAP_R_EXT                                       0x8072
      #define GL_MAX_3D_TEXTURE_SIZE_EXT                                  0x8073

   #endif /* GL_PACK_SKIP_IMAGES_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glTexImage3DEXT                                  OGLEXT_MAKEGLNAME(TexImage3DEXT)
      #define glTexSubImage3DEXT                               OGLEXT_MAKEGLNAME(TexSubImage3DEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glTexImage3DEXT(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glTexSubImage3DEXT(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_texture3D */


/* ---[ GL_EXT_texture_compression_s3tc ]-------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_compression_s3tc.txt */

#ifndef GL_EXT_texture_compression_s3tc

   #define GL_EXT_texture_compression_s3tc 1
   #define GL_EXT_texture_compression_s3tc_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT

      #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                             0x83f0
      #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                            0x83f1
      #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                            0x83f2
      #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                            0x83f3

   #endif /* GL_COMPRESSED_RGB_S3TC_DXT1_EXT */

#endif /* GL_EXT_texture_compression_s3tc */


/* ---[ GL_EXT_texture_env_add ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_env_add.txt */

#ifndef GL_EXT_texture_env_add

   #define GL_EXT_texture_env_add 1
   #define GL_EXT_texture_env_add_OGLEXT 1

#endif /* GL_EXT_texture_env_add */


/* ---[ GL_EXT_texture_env_combine ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_env_combine.txt */

#ifndef GL_EXT_texture_env_combine

   #define GL_EXT_texture_env_combine 1
   #define GL_EXT_texture_env_combine_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COMBINE_EXT

      #define GL_COMBINE_EXT                                              0x8570
      #define GL_COMBINE_RGB_EXT                                          0x8571
      #define GL_COMBINE_ALPHA_EXT                                        0x8572
      #define GL_RGB_SCALE_EXT                                            0x8573
      #define GL_ADD_SIGNED_EXT                                           0x8574
      #define GL_INTERPOLATE_EXT                                          0x8575
      #define GL_CONSTANT_EXT                                             0x8576
      #define GL_PRIMARY_COLOR_EXT                                        0x8577
      #define GL_PREVIOUS_EXT                                             0x8578
      #define GL_SOURCE0_RGB_EXT                                          0x8580
      #define GL_SOURCE1_RGB_EXT                                          0x8581
      #define GL_SOURCE2_RGB_EXT                                          0x8582
      #define GL_SOURCE0_ALPHA_EXT                                        0x8588
      #define GL_SOURCE1_ALPHA_EXT                                        0x8589
      #define GL_SOURCE2_ALPHA_EXT                                        0x858a
      #define GL_OPERAND0_RGB_EXT                                         0x8590
      #define GL_OPERAND1_RGB_EXT                                         0x8591
      #define GL_OPERAND2_RGB_EXT                                         0x8592
      #define GL_OPERAND0_ALPHA_EXT                                       0x8598
      #define GL_OPERAND1_ALPHA_EXT                                       0x8599
      #define GL_OPERAND2_ALPHA_EXT                                       0x859a

   #endif /* GL_COMBINE_EXT */

#endif /* GL_EXT_texture_env_combine */


/* ---[ GL_EXT_texture_env_dot3 ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_env_dot3.txt */

#ifndef GL_EXT_texture_env_dot3

   #define GL_EXT_texture_env_dot3 1
   #define GL_EXT_texture_env_dot3_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DOT3_RGB_EXT

      #define GL_DOT3_RGB_EXT                                             0x8740
      #define GL_DOT3_RGBA_EXT                                            0x8741

   #endif /* GL_DOT3_RGB_EXT */

#endif /* GL_EXT_texture_env_dot3 */


/* ---[ GL_EXT_texture_filter_anisotropic ]------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_filter_anisotropic.txt */

#ifndef GL_EXT_texture_filter_anisotropic

   #define GL_EXT_texture_filter_anisotropic 1
   #define GL_EXT_texture_filter_anisotropic_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT

      #define GL_TEXTURE_MAX_ANISOTROPY_EXT                               0x84fe
      #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                           0x84ff

   #endif /* GL_TEXTURE_MAX_ANISOTROPY_EXT */

#endif /* GL_EXT_texture_filter_anisotropic */


/* ---[ GL_EXT_texture_lod_bias ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_lod_bias.txt */

#ifndef GL_EXT_texture_lod_bias

   #define GL_EXT_texture_lod_bias 1
   #define GL_EXT_texture_lod_bias_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MAX_TEXTURE_LOD_BIAS_EXT

      #define GL_MAX_TEXTURE_LOD_BIAS_EXT                                 0x84fd
      #define GL_TEXTURE_FILTER_CONTROL_EXT                               0x8500
      #define GL_TEXTURE_LOD_BIAS_EXT                                     0x8501

   #endif /* GL_MAX_TEXTURE_LOD_BIAS_EXT */

#endif /* GL_EXT_texture_lod_bias */


/* ---[ GL_EXT_texture_mirror_clamp ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_mirror_clamp.txt */

#ifndef GL_EXT_texture_mirror_clamp

   #define GL_EXT_texture_mirror_clamp 1
   #define GL_EXT_texture_mirror_clamp_OGLEXT 1

#endif /* GL_EXT_texture_mirror_clamp */


/* ---[ GL_EXT_texture_object ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_object.txt */

#ifndef GL_EXT_texture_object

   #define GL_EXT_texture_object 1
   #define GL_EXT_texture_object_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_PRIORITY_EXT

      #define GL_TEXTURE_PRIORITY_EXT                                     0x8066
      #define GL_TEXTURE_RESIDENT_EXT                                     0x8067
      #define GL_TEXTURE_1D_BINDING_EXT                                   0x8068
      #define GL_TEXTURE_2D_BINDING_EXT                                   0x8069
      #define GL_TEXTURE_3D_BINDING_EXT                                   0x806a

   #endif /* GL_TEXTURE_PRIORITY_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glAreTexturesResidentEXT                         OGLEXT_MAKEGLNAME(AreTexturesResidentEXT)
      #define glBindTextureEXT                                 OGLEXT_MAKEGLNAME(BindTextureEXT)
      #define glDeleteTexturesEXT                              OGLEXT_MAKEGLNAME(DeleteTexturesEXT)
      #define glGenTexturesEXT                                 OGLEXT_MAKEGLNAME(GenTexturesEXT)
      #define glIsTextureEXT                                   OGLEXT_MAKEGLNAME(IsTextureEXT)
      #define glPrioritizeTexturesEXT                          OGLEXT_MAKEGLNAME(PrioritizeTexturesEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLboolean         glAreTexturesResidentEXT(GLsizei, GLuint const *, GLboolean *);
      GLAPI GLvoid            glBindTextureEXT(GLenum, GLuint);
      GLAPI GLvoid            glDeleteTexturesEXT(GLsizei, GLuint const *);
      GLAPI GLvoid            glGenTexturesEXT(GLsizei, GLuint *);
      GLAPI GLboolean         glIsTextureEXT(GLuint);
      GLAPI GLvoid            glPrioritizeTexturesEXT(GLsizei, GLuint const *, GLclampf const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_texture_object */


/* ---[ GL_EXT_texture_perturb_normal ]---------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/texture_perturb_normal.txt */

#ifndef GL_EXT_texture_perturb_normal

   #define GL_EXT_texture_perturb_normal 1
   #define GL_EXT_texture_perturb_normal_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PERTURB_EXT

      #define GL_PERTURB_EXT                                              0x85ae
      #define GL_TEXTURE_NORMAL_EXT                                       0x85af

   #endif /* GL_PERTURB_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glTextureNormalEXT                               OGLEXT_MAKEGLNAME(TextureNormalEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glTextureNormalEXT(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_texture_perturb_normal */


/* ---[ GL_EXT_vertex_array ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/vertex_array.txt */

#ifndef GL_EXT_vertex_array

   #define GL_EXT_vertex_array 1
   #define GL_EXT_vertex_array_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_ARRAY_EXT

      #define GL_VERTEX_ARRAY_EXT                                         0x8074
      #define GL_NORMAL_ARRAY_EXT                                         0x8075
      #define GL_COLOR_ARRAY_EXT                                          0x8076
      #define GL_INDEX_ARRAY_EXT                                          0x8077
      #define GL_TEXTURE_COORD_ARRAY_EXT                                  0x8078
      #define GL_EDGE_FLAG_ARRAY_EXT                                      0x8079
      #define GL_VERTEX_ARRAY_SIZE_EXT                                    0x807a
      #define GL_VERTEX_ARRAY_TYPE_EXT                                    0x807b
      #define GL_VERTEX_ARRAY_STRIDE_EXT                                  0x807c
      #define GL_VERTEX_ARRAY_COUNT_EXT                                   0x807d
      #define GL_NORMAL_ARRAY_TYPE_EXT                                    0x807e
      #define GL_NORMAL_ARRAY_STRIDE_EXT                                  0x807f
      #define GL_NORMAL_ARRAY_COUNT_EXT                                   0x8080
      #define GL_COLOR_ARRAY_SIZE_EXT                                     0x8081
      #define GL_COLOR_ARRAY_TYPE_EXT                                     0x8082
      #define GL_COLOR_ARRAY_STRIDE_EXT                                   0x8083
      #define GL_COLOR_ARRAY_COUNT_EXT                                    0x8084
      #define GL_INDEX_ARRAY_TYPE_EXT                                     0x8085
      #define GL_INDEX_ARRAY_STRIDE_EXT                                   0x8086
      #define GL_INDEX_ARRAY_COUNT_EXT                                    0x8087
      #define GL_TEXTURE_COORD_ARRAY_SIZE_EXT                             0x8088
      #define GL_TEXTURE_COORD_ARRAY_TYPE_EXT                             0x8089
      #define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT                           0x808a
      #define GL_TEXTURE_COORD_ARRAY_COUNT_EXT                            0x808b
      #define GL_EDGE_FLAG_ARRAY_STRIDE_EXT                               0x808c
      #define GL_EDGE_FLAG_ARRAY_COUNT_EXT                                0x808d
      #define GL_VERTEX_ARRAY_POINTER_EXT                                 0x808e
      #define GL_NORMAL_ARRAY_POINTER_EXT                                 0x808f
      #define GL_COLOR_ARRAY_POINTER_EXT                                  0x8090
      #define GL_INDEX_ARRAY_POINTER_EXT                                  0x8091
      #define GL_TEXTURE_COORD_ARRAY_POINTER_EXT                          0x8092
      #define GL_EDGE_FLAG_ARRAY_POINTER_EXT                              0x8093

   #endif /* GL_VERTEX_ARRAY_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glArrayElementEXT                                OGLEXT_MAKEGLNAME(ArrayElementEXT)
      #define glColorPointerEXT                                OGLEXT_MAKEGLNAME(ColorPointerEXT)
      #define glDrawArraysEXT                                  OGLEXT_MAKEGLNAME(DrawArraysEXT)
      #define glEdgeFlagPointerEXT                             OGLEXT_MAKEGLNAME(EdgeFlagPointerEXT)
      #define glGetPointervEXT                                 OGLEXT_MAKEGLNAME(GetPointervEXT)
      #define glIndexPointerEXT                                OGLEXT_MAKEGLNAME(IndexPointerEXT)
      #define glNormalPointerEXT                               OGLEXT_MAKEGLNAME(NormalPointerEXT)
      #define glTexCoordPointerEXT                             OGLEXT_MAKEGLNAME(TexCoordPointerEXT)
      #define glVertexPointerEXT                               OGLEXT_MAKEGLNAME(VertexPointerEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glArrayElementEXT(GLint);
      GLAPI GLvoid            glColorPointerEXT(GLint, GLenum, GLsizei, GLsizei, GLvoid const *);
      GLAPI GLvoid            glDrawArraysEXT(GLenum, GLint, GLsizei);
      GLAPI GLvoid            glEdgeFlagPointerEXT(GLsizei, GLsizei, GLboolean const);
      GLAPI GLvoid            glGetPointervEXT(GLenum, GLvoid * *);
      GLAPI GLvoid            glIndexPointerEXT(GLenum, GLsizei, GLsizei, GLvoid const *);
      GLAPI GLvoid            glNormalPointerEXT(GLenum, GLsizei, GLsizei, GLvoid const *);
      GLAPI GLvoid            glTexCoordPointerEXT(GLint, GLenum, GLsizei, GLsizei, GLvoid const *);
      GLAPI GLvoid            glVertexPointerEXT(GLint, GLenum, GLsizei, GLsizei, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_vertex_array */


/* ---[ GL_EXT_vertex_shader ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/vertex_shader.txt */

#ifndef GL_EXT_vertex_shader

   #define GL_EXT_vertex_shader 1
   #define GL_EXT_vertex_shader_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_SHADER_EXT

      #define GL_VERTEX_SHADER_EXT                                        0x8780
      #define GL_VERTEX_SHADER_BINDING_EXT                                0x8781
      #define GL_OP_INDEX_EXT                                             0x8782
      #define GL_OP_NEGATE_EXT                                            0x8783
      #define GL_OP_DOT3_EXT                                              0x8784
      #define GL_OP_DOT4_EXT                                              0x8785
      #define GL_OP_MUL_EXT                                               0x8786
      #define GL_OP_ADD_EXT                                               0x8787
      #define GL_OP_MADD_EXT                                              0x8788
      #define GL_OP_FRAC_EXT                                              0x8789
      #define GL_OP_MAX_EXT                                               0x878a
      #define GL_OP_MIN_EXT                                               0x878b
      #define GL_OP_SET_GE_EXT                                            0x878c
      #define GL_OP_SET_LT_EXT                                            0x878d
      #define GL_OP_CLAMP_EXT                                             0x878e
      #define GL_OP_FLOOR_EXT                                             0x878f
      #define GL_OP_ROUND_EXT                                             0x8790
      #define GL_OP_EXP_BASE_2_EXT                                        0x8791
      #define GL_OP_LOG_BASE_2_EXT                                        0x8792
      #define GL_OP_POWER_EXT                                             0x8793
      #define GL_OP_RECIP_EXT                                             0x8794
      #define GL_OP_RECIP_SQRT_EXT                                        0x8795
      #define GL_OP_SUB_EXT                                               0x8796
      #define GL_OP_CROSS_PRODUCT_EXT                                     0x8797
      #define GL_OP_MULTIPLY_MATRIX_EXT                                   0x8798
      #define GL_OP_MOV_EXT                                               0x8799
      #define GL_OUTPUT_VERTEX_EXT                                        0x879a
      #define GL_OUTPUT_COLOR0_EXT                                        0x879b
      #define GL_OUTPUT_COLOR1_EXT                                        0x879c
      #define GL_OUTPUT_TEXTURE_COORD0_EXT                                0x879d
      #define GL_OUTPUT_TEXTURE_COORD1_EXT                                0x879e
      #define GL_OUTPUT_TEXTURE_COORD2_EXT                                0x879f
      #define GL_OUTPUT_TEXTURE_COORD3_EXT                                0x87a0
      #define GL_OUTPUT_TEXTURE_COORD4_EXT                                0x87a1
      #define GL_OUTPUT_TEXTURE_COORD5_EXT                                0x87a2
      #define GL_OUTPUT_TEXTURE_COORD6_EXT                                0x87a3
      #define GL_OUTPUT_TEXTURE_COORD7_EXT                                0x87a4
      #define GL_OUTPUT_TEXTURE_COORD8_EXT                                0x87a5
      #define GL_OUTPUT_TEXTURE_COORD9_EXT                                0x87a6
      #define GL_OUTPUT_TEXTURE_COORD10_EXT                               0x87a7
      #define GL_OUTPUT_TEXTURE_COORD11_EXT                               0x87a8
      #define GL_OUTPUT_TEXTURE_COORD12_EXT                               0x87a9
      #define GL_OUTPUT_TEXTURE_COORD13_EXT                               0x87aa
      #define GL_OUTPUT_TEXTURE_COORD14_EXT                               0x87ab
      #define GL_OUTPUT_TEXTURE_COORD15_EXT                               0x87ac
      #define GL_OUTPUT_TEXTURE_COORD16_EXT                               0x87ad
      #define GL_OUTPUT_TEXTURE_COORD17_EXT                               0x87ae
      #define GL_OUTPUT_TEXTURE_COORD18_EXT                               0x87af
      #define GL_OUTPUT_TEXTURE_COORD19_EXT                               0x87b0
      #define GL_OUTPUT_TEXTURE_COORD20_EXT                               0x87b1
      #define GL_OUTPUT_TEXTURE_COORD21_EXT                               0x87b2
      #define GL_OUTPUT_TEXTURE_COORD22_EXT                               0x87b3
      #define GL_OUTPUT_TEXTURE_COORD23_EXT                               0x87b4
      #define GL_OUTPUT_TEXTURE_COORD24_EXT                               0x87b5
      #define GL_OUTPUT_TEXTURE_COORD25_EXT                               0x87b6
      #define GL_OUTPUT_TEXTURE_COORD26_EXT                               0x87b7
      #define GL_OUTPUT_TEXTURE_COORD27_EXT                               0x87b8
      #define GL_OUTPUT_TEXTURE_COORD28_EXT                               0x87b9
      #define GL_OUTPUT_TEXTURE_COORD29_EXT                               0x87ba
      #define GL_OUTPUT_TEXTURE_COORD30_EXT                               0x87bb
      #define GL_OUTPUT_TEXTURE_COORD31_EXT                               0x87bc
      #define GL_OUTPUT_FOG_EXT                                           0x87bd
      #define GL_SCALAR_EXT                                               0x87be
      #define GL_VECTOR_EXT                                               0x87bf
      #define GL_MATRIX_EXT                                               0x87c0
      #define GL_VARIANT_EXT                                              0x87c1
      #define GL_INVARIANT_EXT                                            0x87c2
      #define GL_LOCAL_CONSTANT_EXT                                       0x87c3
      #define GL_LOCAL_EXT                                                0x87c4
      #define GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT                       0x87c5
      #define GL_MAX_VERTEX_SHADER_VARIANTS_EXT                           0x87c6
      #define GL_MAX_VERTEX_SHADER_INVARIANTS_EXT                         0x87c7
      #define GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                    0x87c8
      #define GL_MAX_VERTEX_SHADER_LOCALS_EXT                             0x87c9
      #define GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT             0x87ca
      #define GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT                 0x87cb
      #define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT          0x87cc
      #define GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT               0x87cd
      #define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT                   0x87ce
      #define GL_VERTEX_SHADER_INSTRUCTIONS_EXT                           0x87cf
      #define GL_VERTEX_SHADER_VARIANTS_EXT                               0x87d0
      #define GL_VERTEX_SHADER_INVARIANTS_EXT                             0x87d1
      #define GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                        0x87d2
      #define GL_VERTEX_SHADER_LOCALS_EXT                                 0x87d3
      #define GL_VERTEX_SHADER_OPTIMIZED_EXT                              0x87d4
      #define GL_X_EXT                                                    0x87d5
      #define GL_Y_EXT                                                    0x87d6
      #define GL_Z_EXT                                                    0x87d7
      #define GL_W_EXT                                                    0x87d8
      #define GL_NEGATIVE_X_EXT                                           0x87d9
      #define GL_NEGATIVE_Y_EXT                                           0x87da
      #define GL_NEGATIVE_Z_EXT                                           0x87db
      #define GL_NEGATIVE_W_EXT                                           0x87dc
      #define GL_ZERO_EXT                                                 0x87dd
      #define GL_ONE_EXT                                                  0x87de
      #define GL_NEGATIVE_ONE_EXT                                         0x87df
      #define GL_NORMALIZED_RANGE_EXT                                     0x87e0
      #define GL_FULL_RANGE_EXT                                           0x87e1
      #define GL_CURRENT_VERTEX_EXT                                       0x87e2
      #define GL_MVP_MATRIX_EXT                                           0x87e3
      #define GL_VARIANT_VALUE_EXT                                        0x87e4
      #define GL_VARIANT_DATATYPE_EXT                                     0x87e5
      #define GL_VARIANT_ARRAY_STRIDE_EXT                                 0x87e6
      #define GL_VARIANT_ARRAY_TYPE_EXT                                   0x87e7
      #define GL_VARIANT_ARRAY_EXT                                        0x87e8
      #define GL_VARIANT_ARRAY_POINTER_EXT                                0x87e9
      #define GL_INVARIANT_VALUE_EXT                                      0x87ea
      #define GL_INVARIANT_DATATYPE_EXT                                   0x87eb
      #define GL_LOCAL_CONSTANT_VALUE_EXT                                 0x87ec
      #define GL_LOCAL_CONSTANT_DATATYPE_EXT                              0x87ed

   #endif /* GL_VERTEX_SHADER_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBeginVertexShaderEXT                           OGLEXT_MAKEGLNAME(BeginVertexShaderEXT)
      #define glBindLightParameterEXT                          OGLEXT_MAKEGLNAME(BindLightParameterEXT)
      #define glBindMaterialParameterEXT                       OGLEXT_MAKEGLNAME(BindMaterialParameterEXT)
      #define glBindParameterEXT                               OGLEXT_MAKEGLNAME(BindParameterEXT)
      #define glBindTexGenParameterEXT                         OGLEXT_MAKEGLNAME(BindTexGenParameterEXT)
      #define glBindTextureUnitParameterEXT                    OGLEXT_MAKEGLNAME(BindTextureUnitParameterEXT)
      #define glBindVertexShaderEXT                            OGLEXT_MAKEGLNAME(BindVertexShaderEXT)
      #define glDeleteVertexShaderEXT                          OGLEXT_MAKEGLNAME(DeleteVertexShaderEXT)
      #define glDisableVariantClientStateEXT                   OGLEXT_MAKEGLNAME(DisableVariantClientStateEXT)
      #define glEnableVariantClientStateEXT                    OGLEXT_MAKEGLNAME(EnableVariantClientStateEXT)
      #define glEndVertexShaderEXT                             OGLEXT_MAKEGLNAME(EndVertexShaderEXT)
      #define glExtractComponentEXT                            OGLEXT_MAKEGLNAME(ExtractComponentEXT)
      #define glGenSymbolsEXT                                  OGLEXT_MAKEGLNAME(GenSymbolsEXT)
      #define glGenVertexShadersEXT                            OGLEXT_MAKEGLNAME(GenVertexShadersEXT)
      #define glGetInvariantBooleanvEXT                        OGLEXT_MAKEGLNAME(GetInvariantBooleanvEXT)
      #define glGetInvariantFloatvEXT                          OGLEXT_MAKEGLNAME(GetInvariantFloatvEXT)
      #define glGetInvariantIntegervEXT                        OGLEXT_MAKEGLNAME(GetInvariantIntegervEXT)
      #define glGetLocalConstantBooleanvEXT                    OGLEXT_MAKEGLNAME(GetLocalConstantBooleanvEXT)
      #define glGetLocalConstantFloatvEXT                      OGLEXT_MAKEGLNAME(GetLocalConstantFloatvEXT)
      #define glGetLocalConstantIntegervEXT                    OGLEXT_MAKEGLNAME(GetLocalConstantIntegervEXT)
      #define glGetVariantBooleanvEXT                          OGLEXT_MAKEGLNAME(GetVariantBooleanvEXT)
      #define glGetVariantFloatvEXT                            OGLEXT_MAKEGLNAME(GetVariantFloatvEXT)
      #define glGetVariantIntegervEXT                          OGLEXT_MAKEGLNAME(GetVariantIntegervEXT)
      #define glGetVariantPointervEXT                          OGLEXT_MAKEGLNAME(GetVariantPointervEXT)
      #define glInsertComponentEXT                             OGLEXT_MAKEGLNAME(InsertComponentEXT)
      #define glIsVariantEnabledEXT                            OGLEXT_MAKEGLNAME(IsVariantEnabledEXT)
      #define glSetInvariantEXT                                OGLEXT_MAKEGLNAME(SetInvariantEXT)
      #define glSetLocalConstantEXT                            OGLEXT_MAKEGLNAME(SetLocalConstantEXT)
      #define glShaderOp1EXT                                   OGLEXT_MAKEGLNAME(ShaderOp1EXT)
      #define glShaderOp2EXT                                   OGLEXT_MAKEGLNAME(ShaderOp2EXT)
      #define glShaderOp3EXT                                   OGLEXT_MAKEGLNAME(ShaderOp3EXT)
      #define glSwizzleEXT                                     OGLEXT_MAKEGLNAME(SwizzleEXT)
      #define glVariantbvEXT                                   OGLEXT_MAKEGLNAME(VariantbvEXT)
      #define glVariantdvEXT                                   OGLEXT_MAKEGLNAME(VariantdvEXT)
      #define glVariantfvEXT                                   OGLEXT_MAKEGLNAME(VariantfvEXT)
      #define glVariantivEXT                                   OGLEXT_MAKEGLNAME(VariantivEXT)
      #define glVariantPointerEXT                              OGLEXT_MAKEGLNAME(VariantPointerEXT)
      #define glVariantsvEXT                                   OGLEXT_MAKEGLNAME(VariantsvEXT)
      #define glVariantubvEXT                                  OGLEXT_MAKEGLNAME(VariantubvEXT)
      #define glVariantuivEXT                                  OGLEXT_MAKEGLNAME(VariantuivEXT)
      #define glVariantusvEXT                                  OGLEXT_MAKEGLNAME(VariantusvEXT)
      #define glWriteMaskEXT                                   OGLEXT_MAKEGLNAME(WriteMaskEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBeginVertexShaderEXT();
      GLAPI GLuint            glBindLightParameterEXT(GLenum, GLenum);
      GLAPI GLuint            glBindMaterialParameterEXT(GLenum, GLenum);
      GLAPI GLuint            glBindParameterEXT(GLenum);
      GLAPI GLuint            glBindTexGenParameterEXT(GLenum, GLenum, GLenum);
      GLAPI GLuint            glBindTextureUnitParameterEXT(GLenum, GLenum);
      GLAPI GLvoid            glBindVertexShaderEXT(GLuint);
      GLAPI GLvoid            glDeleteVertexShaderEXT(GLuint);
      GLAPI GLvoid            glDisableVariantClientStateEXT(GLuint);
      GLAPI GLvoid            glEnableVariantClientStateEXT(GLuint);
      GLAPI GLvoid            glEndVertexShaderEXT();
      GLAPI GLvoid            glExtractComponentEXT(GLuint, GLuint, GLuint);
      GLAPI GLuint            glGenSymbolsEXT(GLenum, GLenum, GLenum, GLuint);
      GLAPI GLuint            glGenVertexShadersEXT(GLuint);
      GLAPI GLvoid            glGetInvariantBooleanvEXT(GLuint, GLenum, GLboolean *);
      GLAPI GLvoid            glGetInvariantFloatvEXT(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetInvariantIntegervEXT(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetLocalConstantBooleanvEXT(GLuint, GLenum, GLboolean *);
      GLAPI GLvoid            glGetLocalConstantFloatvEXT(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetLocalConstantIntegervEXT(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetVariantBooleanvEXT(GLuint, GLenum, GLboolean *);
      GLAPI GLvoid            glGetVariantFloatvEXT(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetVariantIntegervEXT(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetVariantPointervEXT(GLuint, GLenum, GLvoid * *);
      GLAPI GLvoid            glInsertComponentEXT(GLuint, GLuint, GLuint);
      GLAPI GLboolean         glIsVariantEnabledEXT(GLuint, GLenum);
      GLAPI GLvoid            glSetInvariantEXT(GLuint, GLenum, GLvoid const *);
      GLAPI GLvoid            glSetLocalConstantEXT(GLuint, GLenum, GLvoid const *);
      GLAPI GLvoid            glShaderOp1EXT(GLenum, GLuint, GLuint);
      GLAPI GLvoid            glShaderOp2EXT(GLenum, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glShaderOp3EXT(GLenum, GLuint, GLuint, GLuint, GLuint);
      GLAPI GLvoid            glSwizzleEXT(GLuint, GLuint, GLenum, GLenum, GLenum, GLenum);
      GLAPI GLvoid            glVariantbvEXT(GLuint, GLbyte const *);
      GLAPI GLvoid            glVariantdvEXT(GLuint, GLdouble const *);
      GLAPI GLvoid            glVariantfvEXT(GLuint, GLfloat const *);
      GLAPI GLvoid            glVariantivEXT(GLuint, GLint const *);
      GLAPI GLvoid            glVariantPointerEXT(GLuint, GLenum, GLuint, GLvoid const *);
      GLAPI GLvoid            glVariantsvEXT(GLuint, GLshort const *);
      GLAPI GLvoid            glVariantubvEXT(GLuint, GLubyte const *);
      GLAPI GLvoid            glVariantuivEXT(GLuint, GLuint const *);
      GLAPI GLvoid            glVariantusvEXT(GLuint, GLushort const *);
      GLAPI GLvoid            glWriteMaskEXT(GLuint, GLuint, GLenum, GLenum, GLenum, GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_vertex_shader */


/* ---[ GL_EXT_vertex_weighting ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/EXT/vertex_weighting.txt */

#ifndef GL_EXT_vertex_weighting

   #define GL_EXT_vertex_weighting 1
   #define GL_EXT_vertex_weighting_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MODELVIEW0_STACK_DEPTH_EXT

      #define GL_MODELVIEW0_STACK_DEPTH_EXT                               0xba3
      #define GL_MODELVIEW0_MATRIX_EXT                                    0xba6
      #define GL_MODELVIEW0_EXT                                           0x1700
      #define GL_MODELVIEW1_STACK_DEPTH_EXT                               0x8502
      #define GL_MODELVIEW1_MATRIX_EXT                                    0x8506
      #define GL_VERTEX_WEIGHTING_EXT                                     0x8509
      #define GL_MODELVIEW1_EXT                                           0x850a
      #define GL_CURRENT_VERTEX_WEIGHT_EXT                                0x850b
      #define GL_VERTEX_WEIGHT_ARRAY_EXT                                  0x850c
      #define GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT                             0x850d
      #define GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT                             0x850e
      #define GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT                           0x850f
      #define GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT                          0x8510

   #endif /* GL_MODELVIEW0_STACK_DEPTH_EXT */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glVertexWeightfEXT                               OGLEXT_MAKEGLNAME(VertexWeightfEXT)
      #define glVertexWeightfvEXT                              OGLEXT_MAKEGLNAME(VertexWeightfvEXT)
      #define glVertexWeightPointerEXT                         OGLEXT_MAKEGLNAME(VertexWeightPointerEXT)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glVertexWeightfEXT(GLfloat);
      GLAPI GLvoid            glVertexWeightfvEXT(GLfloat const *);
      GLAPI GLvoid            glVertexWeightPointerEXT(GLsizei, GLenum, GLsizei, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_EXT_vertex_weighting */


/* ---[ GL_HP_convolution_border_modes ]--------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/HP/convolution_border_modes.txt */

#ifndef GL_HP_convolution_border_modes

   #define GL_HP_convolution_border_modes 1
   #define GL_HP_convolution_border_modes_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_IGNORE_BORDER_HP

      #define GL_IGNORE_BORDER_HP                                         0x8150
      #define GL_CONSTANT_BORDER_HP                                       0x8151
      #define GL_REPLICATE_BORDER_HP                                      0x8153
      #define GL_CONVOLUTION_BORDER_COLOR_HP                              0x8154

   #endif /* GL_IGNORE_BORDER_HP */

#endif /* GL_HP_convolution_border_modes */


/* ---[ GL_HP_image_transform ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/HP/image_transform.txt */

#ifndef GL_HP_image_transform

   #define GL_HP_image_transform 1
   #define GL_HP_image_transform_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_IMAGE_SCALE_X_HP

      #define GL_IMAGE_SCALE_X_HP                                         0x8155
      #define GL_IMAGE_SCALE_Y_HP                                         0x8156
      #define GL_IMAGE_TRANSLATE_X_HP                                     0x8157
      #define GL_IMAGE_TRANSLATE_Y_HP                                     0x8158
      #define GL_IMAGE_ROTATE_ANGLE_HP                                    0x8159
      #define GL_IMAGE_ROTATE_ORIGIN_X_HP                                 0x815a
      #define GL_IMAGE_ROTATE_ORIGIN_Y_HP                                 0x815b
      #define GL_IMAGE_MAG_FILTER_HP                                      0x815c
      #define GL_IMAGE_MIN_FILTER_HP                                      0x815d
      #define GL_IMAGE_CUBIC_WEIGHT_HP                                    0x815e
      #define GL_CUBIC_HP                                                 0x815f
      #define GL_AVERAGE_HP                                               0x8160
      #define GL_IMAGE_TRANSFORM_2D_HP                                    0x8161
      #define GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP                      0x8162
      #define GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP                0x8163

   #endif /* GL_IMAGE_SCALE_X_HP */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetImageTransformParameterfvHP                 OGLEXT_MAKEGLNAME(GetImageTransformParameterfvHP)
      #define glGetImageTransformParameterivHP                 OGLEXT_MAKEGLNAME(GetImageTransformParameterivHP)
      #define glImageTransformParameterfHP                     OGLEXT_MAKEGLNAME(ImageTransformParameterfHP)
      #define glImageTransformParameterfvHP                    OGLEXT_MAKEGLNAME(ImageTransformParameterfvHP)
      #define glImageTransformParameteriHP                     OGLEXT_MAKEGLNAME(ImageTransformParameteriHP)
      #define glImageTransformParameterivHP                    OGLEXT_MAKEGLNAME(ImageTransformParameterivHP)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetImageTransformParameterfvHP(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetImageTransformParameterivHP(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glImageTransformParameterfHP(GLenum, GLenum, GLfloat);
      GLAPI GLvoid            glImageTransformParameterfvHP(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glImageTransformParameteriHP(GLenum, GLenum, GLint);
      GLAPI GLvoid            glImageTransformParameterivHP(GLenum, GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_HP_image_transform */


/* ---[ GL_HP_occlusion_test ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/HP/occlusion_test.txt */

#ifndef GL_HP_occlusion_test

   #define GL_HP_occlusion_test 1
   #define GL_HP_occlusion_test_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_OCCLUSION_TEST_HP

      #define GL_OCCLUSION_TEST_HP                                        0x8165
      #define GL_OCCLUSION_TEST_RESULT_HP                                 0x8166

   #endif /* GL_OCCLUSION_TEST_HP */

#endif /* GL_HP_occlusion_test */


/* ---[ GL_HP_texture_lighting ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/HP/texture_lighting.txt */

#ifndef GL_HP_texture_lighting

   #define GL_HP_texture_lighting 1
   #define GL_HP_texture_lighting_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_LIGHTING_MODE_HP

      #define GL_TEXTURE_LIGHTING_MODE_HP                                 0x8167
      #define GL_TEXTURE_POST_SPECULAR_HP                                 0x8168
      #define GL_TEXTURE_PRE_SPECULAR_HP                                  0x8169

   #endif /* GL_TEXTURE_LIGHTING_MODE_HP */

#endif /* GL_HP_texture_lighting */


/* ---[ GL_IBM_cull_vertex ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/IBM/cull_vertex.txt */

#ifndef GL_IBM_cull_vertex

   #define GL_IBM_cull_vertex 1
   #define GL_IBM_cull_vertex_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CULL_VERTEX_IBM

      #define GL_CULL_VERTEX_IBM                                          0x1928a

   #endif /* GL_CULL_VERTEX_IBM */

#endif /* GL_IBM_cull_vertex */


/* ---[ GL_IBM_multimode_draw_arrays ]----------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/IBM/multimode_draw_arrays.txt */

#ifndef GL_IBM_multimode_draw_arrays

   #define GL_IBM_multimode_draw_arrays 1
   #define GL_IBM_multimode_draw_arrays_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glMultiModeDrawArraysIBM                         OGLEXT_MAKEGLNAME(MultiModeDrawArraysIBM)
      #define glMultiModeDrawElementsIBM                       OGLEXT_MAKEGLNAME(MultiModeDrawElementsIBM)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glMultiModeDrawArraysIBM(GLenum, GLint const *, GLsizei const *, GLsizei, GLint);
      GLAPI GLvoid            glMultiModeDrawElementsIBM(GLenum const *, GLsizei const *, GLenum, GLvoid const * *, GLsizei, GLint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_IBM_multimode_draw_arrays */


/* ---[ GL_IBM_rasterpos_clip ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/IBM/rasterpos_clip.txt */

#ifndef GL_IBM_rasterpos_clip

   #define GL_IBM_rasterpos_clip 1
   #define GL_IBM_rasterpos_clip_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_RASTER_POSITION_UNCLIPPED_IBM

      #define GL_RASTER_POSITION_UNCLIPPED_IBM                            0x19262

   #endif /* GL_RASTER_POSITION_UNCLIPPED_IBM */

#endif /* GL_IBM_rasterpos_clip */


/* ---[ GL_IBM_static_data ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/IBM/static_data.txt */

#ifndef GL_IBM_static_data

   #define GL_IBM_static_data 1
   #define GL_IBM_static_data_OGLEXT 1

#endif /* GL_IBM_static_data */


/* ---[ GL_IBM_texture_mirrored_repeat ]--------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/IBM/texture_mirrored_repeat.txt */

#ifndef GL_IBM_texture_mirrored_repeat

   #define GL_IBM_texture_mirrored_repeat 1
   #define GL_IBM_texture_mirrored_repeat_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MIRRORED_REPEAT_IBM

      #define GL_MIRRORED_REPEAT_IBM                                      0x8370

   #endif /* GL_MIRRORED_REPEAT_IBM */

#endif /* GL_IBM_texture_mirrored_repeat */


/* ---[ GL_IBM_vertex_array_lists ]-------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/IBM/vertex_array_lists.txt */

#ifndef GL_IBM_vertex_array_lists

   #define GL_IBM_vertex_array_lists 1
   #define GL_IBM_vertex_array_lists_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_ARRAY_LIST_IBM

      #define GL_VERTEX_ARRAY_LIST_IBM                                    0x1929e
      #define GL_NORMAL_ARRAY_LIST_IBM                                    0x1929f
      #define GL_COLOR_ARRAY_LIST_IBM                                     0x192a0
      #define GL_INDEX_ARRAY_LIST_IBM                                     0x192a1
      #define GL_TEXTURE_COORD_ARRAY_LIST_IBM                             0x192a2
      #define GL_EDGE_FLAG_ARRAY_LIST_IBM                                 0x192a3
      #define GL_FOG_COORDINATE_ARRAY_LIST_IBM                            0x192a4
      #define GL_SECONDARY_COLOR_ARRAY_LIST_IBM                           0x192a5
      #define GL_VERTEX_ARRAY_LIST_STRIDE_IBM                             0x192a8
      #define GL_NORMAL_ARRAY_LIST_STRIDE_IBM                             0x192a9
      #define GL_COLOR_ARRAY_LIST_STRIDE_IBM                              0x192aa
      #define GL_INDEX_ARRAY_LIST_STRIDE_IBM                              0x192ab
      #define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM                      0x192ac
      #define GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM                          0x192ad
      #define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM                     0x192ae
      #define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM                    0x192af

   #endif /* GL_VERTEX_ARRAY_LIST_IBM */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glColorPointerListIBM                            OGLEXT_MAKEGLNAME(ColorPointerListIBM)
      #define glEdgeFlagPointerListIBM                         OGLEXT_MAKEGLNAME(EdgeFlagPointerListIBM)
      #define glFogCoordPointerListIBM                         OGLEXT_MAKEGLNAME(FogCoordPointerListIBM)
      #define glIndexPointerListIBM                            OGLEXT_MAKEGLNAME(IndexPointerListIBM)
      #define glNormalPointerListIBM                           OGLEXT_MAKEGLNAME(NormalPointerListIBM)
      #define glSecondaryColorPointerListIBM                   OGLEXT_MAKEGLNAME(SecondaryColorPointerListIBM)
      #define glTexCoordPointerListIBM                         OGLEXT_MAKEGLNAME(TexCoordPointerListIBM)
      #define glVertexPointerListIBM                           OGLEXT_MAKEGLNAME(VertexPointerListIBM)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glColorPointerListIBM(GLint, GLenum, GLint, GLvoid const * *, GLint);
      GLAPI GLvoid            glEdgeFlagPointerListIBM(GLint, GLboolean const, GLint);
      GLAPI GLvoid            glFogCoordPointerListIBM(GLenum, GLint, GLvoid const * *, GLint);
      GLAPI GLvoid            glIndexPointerListIBM(GLenum, GLint, GLvoid const * *, GLint);
      GLAPI GLvoid            glNormalPointerListIBM(GLenum, GLint, GLvoid const * *, GLint);
      GLAPI GLvoid            glSecondaryColorPointerListIBM(GLint, GLenum, GLint, GLvoid const * *, GLint);
      GLAPI GLvoid            glTexCoordPointerListIBM(GLint, GLenum, GLint, GLvoid const * *, GLint);
      GLAPI GLvoid            glVertexPointerListIBM(GLint, GLenum, GLint, GLvoid const * *, GLint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_IBM_vertex_array_lists */


/* ---[ GL_INGR_blend_func_separate ]------------------------------------------------------------------------ */

/*! link :  */

#ifndef GL_INGR_blend_func_separate

   #define GL_INGR_blend_func_separate 1
   #define GL_INGR_blend_func_separate_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBlendFuncSeparateINGR                          OGLEXT_MAKEGLNAME(BlendFuncSeparateINGR)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBlendFuncSeparateINGR(GLenum, GLenum, GLenum, GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_INGR_blend_func_separate */


/* ---[ GL_INGR_color_clamp ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/INGR/color_clamp.txt */

#ifndef GL_INGR_color_clamp

   #define GL_INGR_color_clamp 1
   #define GL_INGR_color_clamp_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_RED_MIN_CLAMP_INGR

      #define GL_RED_MIN_CLAMP_INGR                                       0x8560
      #define GL_GREEN_MIN_CLAMP_INGR                                     0x8561
      #define GL_BLUE_MIN_CLAMP_INGR                                      0x8562
      #define GL_ALPHA_MIN_CLAMP_INGR                                     0x8563
      #define GL_RED_MAX_CLAMP_INGR                                       0x8564
      #define GL_GREEN_MAX_CLAMP_INGR                                     0x8565
      #define GL_BLUE_MAX_CLAMP_INGR                                      0x8566
      #define GL_ALPHA_MAX_CLAMP_INGR                                     0x8567

   #endif /* GL_RED_MIN_CLAMP_INGR */

#endif /* GL_INGR_color_clamp */


/* ---[ GL_INGR_interlace_read ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/INGR/interlace_read.txt */

#ifndef GL_INGR_interlace_read

   #define GL_INGR_interlace_read 1
   #define GL_INGR_interlace_read_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_INTERLACE_READ_INGR

      #define GL_INTERLACE_READ_INGR                                      0x8568

   #endif /* GL_INTERLACE_READ_INGR */

#endif /* GL_INGR_interlace_read */


/* ---[ GL_INTEL_parallel_arrays ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/INTEL/parallel_arrays.txt */

#ifndef GL_INTEL_parallel_arrays

   #define GL_INTEL_parallel_arrays 1
   #define GL_INTEL_parallel_arrays_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PARALLEL_ARRAYS_INTEL

      #define GL_PARALLEL_ARRAYS_INTEL                                    0x83f4
      #define GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL                     0x83f5
      #define GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL                     0x83f6
      #define GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL                      0x83f7
      #define GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL              0x83f8

   #endif /* GL_PARALLEL_ARRAYS_INTEL */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glColorPointervINTEL                             OGLEXT_MAKEGLNAME(ColorPointervINTEL)
      #define glNormalPointervINTEL                            OGLEXT_MAKEGLNAME(NormalPointervINTEL)
      #define glTexCoordPointervINTEL                          OGLEXT_MAKEGLNAME(TexCoordPointervINTEL)
      #define glVertexPointervINTEL                            OGLEXT_MAKEGLNAME(VertexPointervINTEL)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glColorPointervINTEL(GLint, GLenum, GLvoid const * *);
      GLAPI GLvoid            glNormalPointervINTEL(GLenum, GLvoid const * *);
      GLAPI GLvoid            glTexCoordPointervINTEL(GLint, GLenum, GLvoid const * *);
      GLAPI GLvoid            glVertexPointervINTEL(GLint, GLenum, GLvoid const * *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_INTEL_parallel_arrays */


/* ---[ GL_MESA_resize_buffers ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/MESA/resize_buffers.txt */

#ifndef GL_MESA_resize_buffers

   #define GL_MESA_resize_buffers 1
   #define GL_MESA_resize_buffers_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glResizeBuffersMESA                              OGLEXT_MAKEGLNAME(ResizeBuffersMESA)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glResizeBuffersMESA();

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_MESA_resize_buffers */


/* ---[ GL_MESA_window_pos ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/MESA/window_pos.txt */

#ifndef GL_MESA_window_pos

   #define GL_MESA_window_pos 1
   #define GL_MESA_window_pos_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glWindowPos2dMESA                                OGLEXT_MAKEGLNAME(WindowPos2dMESA)
      #define glWindowPos2dvMESA                               OGLEXT_MAKEGLNAME(WindowPos2dvMESA)
      #define glWindowPos2fMESA                                OGLEXT_MAKEGLNAME(WindowPos2fMESA)
      #define glWindowPos2fvMESA                               OGLEXT_MAKEGLNAME(WindowPos2fvMESA)
      #define glWindowPos2iMESA                                OGLEXT_MAKEGLNAME(WindowPos2iMESA)
      #define glWindowPos2ivMESA                               OGLEXT_MAKEGLNAME(WindowPos2ivMESA)
      #define glWindowPos2sMESA                                OGLEXT_MAKEGLNAME(WindowPos2sMESA)
      #define glWindowPos2svMESA                               OGLEXT_MAKEGLNAME(WindowPos2svMESA)
      #define glWindowPos3dMESA                                OGLEXT_MAKEGLNAME(WindowPos3dMESA)
      #define glWindowPos3dvMESA                               OGLEXT_MAKEGLNAME(WindowPos3dvMESA)
      #define glWindowPos3fMESA                                OGLEXT_MAKEGLNAME(WindowPos3fMESA)
      #define glWindowPos3fvMESA                               OGLEXT_MAKEGLNAME(WindowPos3fvMESA)
      #define glWindowPos3iMESA                                OGLEXT_MAKEGLNAME(WindowPos3iMESA)
      #define glWindowPos3ivMESA                               OGLEXT_MAKEGLNAME(WindowPos3ivMESA)
      #define glWindowPos3sMESA                                OGLEXT_MAKEGLNAME(WindowPos3sMESA)
      #define glWindowPos3svMESA                               OGLEXT_MAKEGLNAME(WindowPos3svMESA)
      #define glWindowPos4dMESA                                OGLEXT_MAKEGLNAME(WindowPos4dMESA)
      #define glWindowPos4dvMESA                               OGLEXT_MAKEGLNAME(WindowPos4dvMESA)
      #define glWindowPos4fMESA                                OGLEXT_MAKEGLNAME(WindowPos4fMESA)
      #define glWindowPos4fvMESA                               OGLEXT_MAKEGLNAME(WindowPos4fvMESA)
      #define glWindowPos4iMESA                                OGLEXT_MAKEGLNAME(WindowPos4iMESA)
      #define glWindowPos4ivMESA                               OGLEXT_MAKEGLNAME(WindowPos4ivMESA)
      #define glWindowPos4sMESA                                OGLEXT_MAKEGLNAME(WindowPos4sMESA)
      #define glWindowPos4svMESA                               OGLEXT_MAKEGLNAME(WindowPos4svMESA)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glWindowPos2dMESA(GLdouble, GLdouble);
      GLAPI GLvoid            glWindowPos2dvMESA(GLdouble const *);
      GLAPI GLvoid            glWindowPos2fMESA(GLfloat, GLfloat);
      GLAPI GLvoid            glWindowPos2fvMESA(GLfloat const *);
      GLAPI GLvoid            glWindowPos2iMESA(GLint, GLint);
      GLAPI GLvoid            glWindowPos2ivMESA(GLint const *);
      GLAPI GLvoid            glWindowPos2sMESA(GLshort, GLshort);
      GLAPI GLvoid            glWindowPos2svMESA(GLshort const *);
      GLAPI GLvoid            glWindowPos3dMESA(GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glWindowPos3dvMESA(GLdouble const *);
      GLAPI GLvoid            glWindowPos3fMESA(GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glWindowPos3fvMESA(GLfloat const *);
      GLAPI GLvoid            glWindowPos3iMESA(GLint, GLint, GLint);
      GLAPI GLvoid            glWindowPos3ivMESA(GLint const *);
      GLAPI GLvoid            glWindowPos3sMESA(GLshort, GLshort, GLshort);
      GLAPI GLvoid            glWindowPos3svMESA(GLshort const *);
      GLAPI GLvoid            glWindowPos4dMESA(GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glWindowPos4dvMESA(GLdouble const *);
      GLAPI GLvoid            glWindowPos4fMESA(GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glWindowPos4fvMESA(GLfloat const *);
      GLAPI GLvoid            glWindowPos4iMESA(GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glWindowPos4ivMESA(GLint const *);
      GLAPI GLvoid            glWindowPos4sMESA(GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glWindowPos4svMESA(GLshort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_MESA_window_pos */


/* ---[ GL_NV_blend_square ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/blend_square.txt */

#ifndef GL_NV_blend_square

   #define GL_NV_blend_square 1
   #define GL_NV_blend_square_OGLEXT 1

#endif /* GL_NV_blend_square */


/* ---[ GL_NV_copy_depth_to_color ]-------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/copy_depth_to_color.txt */

#ifndef GL_NV_copy_depth_to_color

   #define GL_NV_copy_depth_to_color 1
   #define GL_NV_copy_depth_to_color_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DEPTH_STENCIL_TO_RGBA_NV

      #define GL_DEPTH_STENCIL_TO_RGBA_NV                                 0x886e
      #define GL_DEPTH_STENCIL_TO_BGRA_NV                                 0x886f

   #endif /* GL_DEPTH_STENCIL_TO_RGBA_NV */

#endif /* GL_NV_copy_depth_to_color */


/* ---[ GL_NV_depth_clamp ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/depth_clamp.txt */

#ifndef GL_NV_depth_clamp

   #define GL_NV_depth_clamp 1
   #define GL_NV_depth_clamp_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DEPTH_CLAMP_NV

      #define GL_DEPTH_CLAMP_NV                                           0x864f

   #endif /* GL_DEPTH_CLAMP_NV */

#endif /* GL_NV_depth_clamp */


/* ---[ GL_NV_evaluators ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/evaluators.txt */

#ifndef GL_NV_evaluators

   #define GL_NV_evaluators 1
   #define GL_NV_evaluators_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_EVAL_2D_NV

      #define GL_EVAL_2D_NV                                               0x86c0
      #define GL_EVAL_TRIANGULAR_2D_NV                                    0x86c1
      #define GL_MAP_TESSELLATION_NV                                      0x86c2
      #define GL_MAP_ATTRIB_U_ORDER_NV                                    0x86c3
      #define GL_MAP_ATTRIB_V_ORDER_NV                                    0x86c4
      #define GL_EVAL_FRACTIONAL_TESSELLATION_NV                          0x86c5
      #define GL_EVAL_VERTEX_ATTRIB0_NV                                   0x86c6
      #define GL_EVAL_VERTEX_ATTRIB1_NV                                   0x86c7
      #define GL_EVAL_VERTEX_ATTRIB2_NV                                   0x86c8
      #define GL_EVAL_VERTEX_ATTRIB3_NV                                   0x86c9
      #define GL_EVAL_VERTEX_ATTRIB4_NV                                   0x86ca
      #define GL_EVAL_VERTEX_ATTRIB5_NV                                   0x86cb
      #define GL_EVAL_VERTEX_ATTRIB6_NV                                   0x86cc
      #define GL_EVAL_VERTEX_ATTRIB7_NV                                   0x86cd
      #define GL_EVAL_VERTEX_ATTRIB8_NV                                   0x86ce
      #define GL_EVAL_VERTEX_ATTRIB9_NV                                   0x86cf
      #define GL_EVAL_VERTEX_ATTRIB10_NV                                  0x86d0
      #define GL_EVAL_VERTEX_ATTRIB11_NV                                  0x86d1
      #define GL_EVAL_VERTEX_ATTRIB12_NV                                  0x86d2
      #define GL_EVAL_VERTEX_ATTRIB13_NV                                  0x86d3
      #define GL_EVAL_VERTEX_ATTRIB14_NV                                  0x86d4
      #define GL_EVAL_VERTEX_ATTRIB15_NV                                  0x86d5
      #define GL_MAX_MAP_TESSELLATION_NV                                  0x86d6
      #define GL_MAX_RATIONAL_EVAL_ORDER_NV                               0x86d7

   #endif /* GL_EVAL_2D_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glEvalMapsNV                                     OGLEXT_MAKEGLNAME(EvalMapsNV)
      #define glGetMapAttribParameterfvNV                      OGLEXT_MAKEGLNAME(GetMapAttribParameterfvNV)
      #define glGetMapAttribParameterivNV                      OGLEXT_MAKEGLNAME(GetMapAttribParameterivNV)
      #define glGetMapControlPointsNV                          OGLEXT_MAKEGLNAME(GetMapControlPointsNV)
      #define glGetMapParameterfvNV                            OGLEXT_MAKEGLNAME(GetMapParameterfvNV)
      #define glGetMapParameterivNV                            OGLEXT_MAKEGLNAME(GetMapParameterivNV)
      #define glMapControlPointsNV                             OGLEXT_MAKEGLNAME(MapControlPointsNV)
      #define glMapParameterfvNV                               OGLEXT_MAKEGLNAME(MapParameterfvNV)
      #define glMapParameterivNV                               OGLEXT_MAKEGLNAME(MapParameterivNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glEvalMapsNV(GLenum, GLenum);
      GLAPI GLvoid            glGetMapAttribParameterfvNV(GLenum, GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetMapAttribParameterivNV(GLenum, GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetMapControlPointsNV(GLenum, GLuint, GLenum, GLsizei, GLsizei, GLboolean, GLvoid *);
      GLAPI GLvoid            glGetMapParameterfvNV(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetMapParameterivNV(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glMapControlPointsNV(GLenum, GLuint, GLenum, GLsizei, GLsizei, GLint, GLint, GLboolean, GLvoid const *);
      GLAPI GLvoid            glMapParameterfvNV(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glMapParameterivNV(GLenum, GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_evaluators */


/* ---[ GL_NV_fence ]---------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/fence.txt */

#ifndef GL_NV_fence

   #define GL_NV_fence 1
   #define GL_NV_fence_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ALL_COMPLETED_NV

      #define GL_ALL_COMPLETED_NV                                         0x84f2
      #define GL_FENCE_STATUS_NV                                          0x84f3
      #define GL_FENCE_CONDITION_NV                                       0x84f4

   #endif /* GL_ALL_COMPLETED_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDeleteFencesNV                                 OGLEXT_MAKEGLNAME(DeleteFencesNV)
      #define glFinishFenceNV                                  OGLEXT_MAKEGLNAME(FinishFenceNV)
      #define glGenFencesNV                                    OGLEXT_MAKEGLNAME(GenFencesNV)
      #define glGetFenceivNV                                   OGLEXT_MAKEGLNAME(GetFenceivNV)
      #define glIsFenceNV                                      OGLEXT_MAKEGLNAME(IsFenceNV)
      #define glSetFenceNV                                     OGLEXT_MAKEGLNAME(SetFenceNV)
      #define glTestFenceNV                                    OGLEXT_MAKEGLNAME(TestFenceNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDeleteFencesNV(GLsizei, GLuint const *);
      GLAPI GLvoid            glFinishFenceNV(GLuint);
      GLAPI GLvoid            glGenFencesNV(GLsizei, GLuint *);
      GLAPI GLvoid            glGetFenceivNV(GLuint, GLenum, GLint *);
      GLAPI GLboolean         glIsFenceNV(GLuint);
      GLAPI GLvoid            glSetFenceNV(GLuint, GLenum);
      GLAPI GLboolean         glTestFenceNV(GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_fence */


/* ---[ GL_NV_float_buffer ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/float_buffer.txt */

#ifndef GL_NV_float_buffer

   #define GL_NV_float_buffer 1
   #define GL_NV_float_buffer_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FLOAT_R_NV

      #define GL_FLOAT_R_NV                                               0x8880
      #define GL_FLOAT_RG_NV                                              0x8881
      #define GL_FLOAT_RGB_NV                                             0x8882
      #define GL_FLOAT_RGBA_NV                                            0x8883
      #define GL_FLOAT_R16_NV                                             0x8884
      #define GL_FLOAT_R32_NV                                             0x8885
      #define GL_FLOAT_RG16_NV                                            0x8886
      #define GL_FLOAT_RG32_NV                                            0x8887
      #define GL_FLOAT_RGB16_NV                                           0x8888
      #define GL_FLOAT_RGB32_NV                                           0x8889
      #define GL_FLOAT_RGBA16_NV                                          0x888a
      #define GL_FLOAT_RGBA32_NV                                          0x888b
      #define GL_TEXTURE_FLOAT_COMPONENTS_NV                              0x888c
      #define GL_FLOAT_CLEAR_COLOR_VALUE_NV                               0x888d
      #define GL_FLOAT_RGBA_MODE_NV                                       0x888e

   #endif /* GL_FLOAT_R_NV */

#endif /* GL_NV_float_buffer */


/* ---[ GL_NV_fog_distance ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/fog_distance.txt */

#ifndef GL_NV_fog_distance

   #define GL_NV_fog_distance 1
   #define GL_NV_fog_distance_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FOG_DISTANCE_MODE_NV

      #define GL_FOG_DISTANCE_MODE_NV                                     0x855a
      #define GL_EYE_RADIAL_NV                                            0x855b
      #define GL_EYE_PLANE_ABSOLUTE_NV                                    0x855c

   #endif /* GL_FOG_DISTANCE_MODE_NV */

#endif /* GL_NV_fog_distance */


/* ---[ GL_NV_fragment_program ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/fragment_program.txt */

#ifndef GL_NV_fragment_program

   #define GL_NV_fragment_program 1
   #define GL_NV_fragment_program_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV

      #define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV                 0x8868
      #define GL_FRAGMENT_PROGRAM_NV                                      0x8870
      #define GL_MAX_TEXTURE_COORDS_NV                                    0x8871
      #define GL_MAX_TEXTURE_IMAGE_UNITS_NV                               0x8872
      #define GL_FRAGMENT_PROGRAM_BINDING_NV                              0x8873
      #define GL_PROGRAM_ERROR_STRING_NV                                  0x8874

   #endif /* GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetProgramNamedParameterdvNV                   OGLEXT_MAKEGLNAME(GetProgramNamedParameterdvNV)
      #define glGetProgramNamedParameterfvNV                   OGLEXT_MAKEGLNAME(GetProgramNamedParameterfvNV)
      #define glProgramNamedParameter4dNV                      OGLEXT_MAKEGLNAME(ProgramNamedParameter4dNV)
      #define glProgramNamedParameter4dvNV                     OGLEXT_MAKEGLNAME(ProgramNamedParameter4dvNV)
      #define glProgramNamedParameter4fNV                      OGLEXT_MAKEGLNAME(ProgramNamedParameter4fNV)
      #define glProgramNamedParameter4fvNV                     OGLEXT_MAKEGLNAME(ProgramNamedParameter4fvNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetProgramNamedParameterdvNV(GLuint, GLsizei, GLubyte const *, GLdouble *);
      GLAPI GLvoid            glGetProgramNamedParameterfvNV(GLuint, GLsizei, GLubyte const *, GLfloat *);
      GLAPI GLvoid            glProgramNamedParameter4dNV(GLuint, GLsizei, GLubyte const *, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glProgramNamedParameter4dvNV(GLuint, GLsizei, GLubyte const *, GLdouble const *);
      GLAPI GLvoid            glProgramNamedParameter4fNV(GLuint, GLsizei, GLubyte const *, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glProgramNamedParameter4fvNV(GLuint, GLsizei, GLubyte const *, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_fragment_program */


/* ---[ GL_NV_half_float ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/half_float.txt */

#ifndef GL_NV_half_float

   #define GL_NV_half_float 1
   #define GL_NV_half_float_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_HALF_FLOAT_NV

      #define GL_HALF_FLOAT_NV                                            0x140b

   #endif /* GL_HALF_FLOAT_NV */

   /* - -[ type definitions ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   typedef unsigned short                                                 GLhalfNV;

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glColor3hNV                                      OGLEXT_MAKEGLNAME(Color3hNV)
      #define glColor3hvNV                                     OGLEXT_MAKEGLNAME(Color3hvNV)
      #define glColor4hNV                                      OGLEXT_MAKEGLNAME(Color4hNV)
      #define glColor4hvNV                                     OGLEXT_MAKEGLNAME(Color4hvNV)
      #define glFogCoordhNV                                    OGLEXT_MAKEGLNAME(FogCoordhNV)
      #define glFogCoordhvNV                                   OGLEXT_MAKEGLNAME(FogCoordhvNV)
      #define glMultiTexCoord1hNV                              OGLEXT_MAKEGLNAME(MultiTexCoord1hNV)
      #define glMultiTexCoord1hvNV                             OGLEXT_MAKEGLNAME(MultiTexCoord1hvNV)
      #define glMultiTexCoord2hNV                              OGLEXT_MAKEGLNAME(MultiTexCoord2hNV)
      #define glMultiTexCoord2hvNV                             OGLEXT_MAKEGLNAME(MultiTexCoord2hvNV)
      #define glMultiTexCoord3hNV                              OGLEXT_MAKEGLNAME(MultiTexCoord3hNV)
      #define glMultiTexCoord3hvNV                             OGLEXT_MAKEGLNAME(MultiTexCoord3hvNV)
      #define glMultiTexCoord4hNV                              OGLEXT_MAKEGLNAME(MultiTexCoord4hNV)
      #define glMultiTexCoord4hvNV                             OGLEXT_MAKEGLNAME(MultiTexCoord4hvNV)
      #define glNormal3hNV                                     OGLEXT_MAKEGLNAME(Normal3hNV)
      #define glNormal3hvNV                                    OGLEXT_MAKEGLNAME(Normal3hvNV)
      #define glSecondaryColor3hNV                             OGLEXT_MAKEGLNAME(SecondaryColor3hNV)
      #define glSecondaryColor3hvNV                            OGLEXT_MAKEGLNAME(SecondaryColor3hvNV)
      #define glTexCoord1hNV                                   OGLEXT_MAKEGLNAME(TexCoord1hNV)
      #define glTexCoord1hvNV                                  OGLEXT_MAKEGLNAME(TexCoord1hvNV)
      #define glTexCoord2hNV                                   OGLEXT_MAKEGLNAME(TexCoord2hNV)
      #define glTexCoord2hvNV                                  OGLEXT_MAKEGLNAME(TexCoord2hvNV)
      #define glTexCoord3hNV                                   OGLEXT_MAKEGLNAME(TexCoord3hNV)
      #define glTexCoord3hvNV                                  OGLEXT_MAKEGLNAME(TexCoord3hvNV)
      #define glTexCoord4hNV                                   OGLEXT_MAKEGLNAME(TexCoord4hNV)
      #define glTexCoord4hvNV                                  OGLEXT_MAKEGLNAME(TexCoord4hvNV)
      #define glVertex2hNV                                     OGLEXT_MAKEGLNAME(Vertex2hNV)
      #define glVertex2hvNV                                    OGLEXT_MAKEGLNAME(Vertex2hvNV)
      #define glVertex3hNV                                     OGLEXT_MAKEGLNAME(Vertex3hNV)
      #define glVertex3hvNV                                    OGLEXT_MAKEGLNAME(Vertex3hvNV)
      #define glVertex4hNV                                     OGLEXT_MAKEGLNAME(Vertex4hNV)
      #define glVertex4hvNV                                    OGLEXT_MAKEGLNAME(Vertex4hvNV)
      #define glVertexAttrib1hNV                               OGLEXT_MAKEGLNAME(VertexAttrib1hNV)
      #define glVertexAttrib1hvNV                              OGLEXT_MAKEGLNAME(VertexAttrib1hvNV)
      #define glVertexAttrib2hNV                               OGLEXT_MAKEGLNAME(VertexAttrib2hNV)
      #define glVertexAttrib2hvNV                              OGLEXT_MAKEGLNAME(VertexAttrib2hvNV)
      #define glVertexAttrib3hNV                               OGLEXT_MAKEGLNAME(VertexAttrib3hNV)
      #define glVertexAttrib3hvNV                              OGLEXT_MAKEGLNAME(VertexAttrib3hvNV)
      #define glVertexAttrib4hNV                               OGLEXT_MAKEGLNAME(VertexAttrib4hNV)
      #define glVertexAttrib4hvNV                              OGLEXT_MAKEGLNAME(VertexAttrib4hvNV)
      #define glVertexAttribs1hvNV                             OGLEXT_MAKEGLNAME(VertexAttribs1hvNV)
      #define glVertexAttribs2hvNV                             OGLEXT_MAKEGLNAME(VertexAttribs2hvNV)
      #define glVertexAttribs3hvNV                             OGLEXT_MAKEGLNAME(VertexAttribs3hvNV)
      #define glVertexAttribs4hvNV                             OGLEXT_MAKEGLNAME(VertexAttribs4hvNV)
      #define glVertexWeighthNV                                OGLEXT_MAKEGLNAME(VertexWeighthNV)
      #define glVertexWeighthvNV                               OGLEXT_MAKEGLNAME(VertexWeighthvNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glColor3hNV(GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glColor3hvNV(GLhalfNV const *);
      GLAPI GLvoid            glColor4hNV(GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glColor4hvNV(GLhalfNV const *);
      GLAPI GLvoid            glFogCoordhNV(GLhalfNV);
      GLAPI GLvoid            glFogCoordhvNV(GLhalfNV const *);
      GLAPI GLvoid            glMultiTexCoord1hNV(GLenum, GLhalfNV);
      GLAPI GLvoid            glMultiTexCoord1hvNV(GLenum, GLhalfNV const *);
      GLAPI GLvoid            glMultiTexCoord2hNV(GLenum, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glMultiTexCoord2hvNV(GLenum, GLhalfNV const *);
      GLAPI GLvoid            glMultiTexCoord3hNV(GLenum, GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glMultiTexCoord3hvNV(GLenum, GLhalfNV const *);
      GLAPI GLvoid            glMultiTexCoord4hNV(GLenum, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glMultiTexCoord4hvNV(GLenum, GLhalfNV const *);
      GLAPI GLvoid            glNormal3hNV(GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glNormal3hvNV(GLhalfNV const *);
      GLAPI GLvoid            glSecondaryColor3hNV(GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glSecondaryColor3hvNV(GLhalfNV const *);
      GLAPI GLvoid            glTexCoord1hNV(GLhalfNV);
      GLAPI GLvoid            glTexCoord1hvNV(GLhalfNV const *);
      GLAPI GLvoid            glTexCoord2hNV(GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glTexCoord2hvNV(GLhalfNV const *);
      GLAPI GLvoid            glTexCoord3hNV(GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glTexCoord3hvNV(GLhalfNV const *);
      GLAPI GLvoid            glTexCoord4hNV(GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glTexCoord4hvNV(GLhalfNV const *);
      GLAPI GLvoid            glVertex2hNV(GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glVertex2hvNV(GLhalfNV const *);
      GLAPI GLvoid            glVertex3hNV(GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glVertex3hvNV(GLhalfNV const *);
      GLAPI GLvoid            glVertex4hNV(GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glVertex4hvNV(GLhalfNV const *);
      GLAPI GLvoid            glVertexAttrib1hNV(GLuint, GLhalfNV);
      GLAPI GLvoid            glVertexAttrib1hvNV(GLuint, GLhalfNV const *);
      GLAPI GLvoid            glVertexAttrib2hNV(GLuint, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glVertexAttrib2hvNV(GLuint, GLhalfNV const *);
      GLAPI GLvoid            glVertexAttrib3hNV(GLuint, GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glVertexAttrib3hvNV(GLuint, GLhalfNV const *);
      GLAPI GLvoid            glVertexAttrib4hNV(GLuint, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
      GLAPI GLvoid            glVertexAttrib4hvNV(GLuint, GLhalfNV const *);
      GLAPI GLvoid            glVertexAttribs1hvNV(GLuint, GLsizei, GLhalfNV const *);
      GLAPI GLvoid            glVertexAttribs2hvNV(GLuint, GLsizei, GLhalfNV const *);
      GLAPI GLvoid            glVertexAttribs3hvNV(GLuint, GLsizei, GLhalfNV const *);
      GLAPI GLvoid            glVertexAttribs4hvNV(GLuint, GLsizei, GLhalfNV const *);
      GLAPI GLvoid            glVertexWeighthNV(GLhalfNV);
      GLAPI GLvoid            glVertexWeighthvNV(GLhalfNV const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_half_float */


/* ---[ GL_NV_light_max_exponent ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/light_max_exponent.txt */

#ifndef GL_NV_light_max_exponent

   #define GL_NV_light_max_exponent 1
   #define GL_NV_light_max_exponent_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MAX_SHININESS_NV

      #define GL_MAX_SHININESS_NV                                         0x8504
      #define GL_MAX_SPOT_EXPONENT_NV                                     0x8505

   #endif /* GL_MAX_SHININESS_NV */

#endif /* GL_NV_light_max_exponent */


/* ---[ GL_NV_multisample_filter_hint ]---------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/multisample_filter_hint.txt */

#ifndef GL_NV_multisample_filter_hint

   #define GL_NV_multisample_filter_hint 1
   #define GL_NV_multisample_filter_hint_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MULTISAMPLE_FILTER_HINT_NV

      #define GL_MULTISAMPLE_FILTER_HINT_NV                               0x8534

   #endif /* GL_MULTISAMPLE_FILTER_HINT_NV */

#endif /* GL_NV_multisample_filter_hint */


/* ---[ GL_NV_occlusion_query ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/occlusion_query.txt */

#ifndef GL_NV_occlusion_query

   #define GL_NV_occlusion_query 1
   #define GL_NV_occlusion_query_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PIXEL_COUNTER_BITS_NV

      #define GL_PIXEL_COUNTER_BITS_NV                                    0x8864
      #define GL_CURRENT_OCCLUSION_QUERY_ID_NV                            0x8865
      #define GL_PIXEL_COUNT_NV                                           0x8866
      #define GL_PIXEL_COUNT_AVAILABLE_NV                                 0x8867

   #endif /* GL_PIXEL_COUNTER_BITS_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glBeginOcclusionQueryNV                          OGLEXT_MAKEGLNAME(BeginOcclusionQueryNV)
      #define glDeleteOcclusionQueriesNV                       OGLEXT_MAKEGLNAME(DeleteOcclusionQueriesNV)
      #define glEndOcclusionQueryNV                            OGLEXT_MAKEGLNAME(EndOcclusionQueryNV)
      #define glGenOcclusionQueriesNV                          OGLEXT_MAKEGLNAME(GenOcclusionQueriesNV)
      #define glGetOcclusionQueryivNV                          OGLEXT_MAKEGLNAME(GetOcclusionQueryivNV)
      #define glGetOcclusionQueryuivNV                         OGLEXT_MAKEGLNAME(GetOcclusionQueryuivNV)
      #define glIsOcclusionQueryNV                             OGLEXT_MAKEGLNAME(IsOcclusionQueryNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glBeginOcclusionQueryNV(GLuint);
      GLAPI GLvoid            glDeleteOcclusionQueriesNV(GLsizei, GLuint const *);
      GLAPI GLvoid            glEndOcclusionQueryNV();
      GLAPI GLvoid            glGenOcclusionQueriesNV(GLsizei, GLuint *);
      GLAPI GLvoid            glGetOcclusionQueryivNV(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetOcclusionQueryuivNV(GLuint, GLenum, GLuint *);
      GLAPI GLboolean         glIsOcclusionQueryNV(GLuint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_occlusion_query */


/* ---[ GL_NV_packed_depth_stencil ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/packed_depth_stencil.txt */

#ifndef GL_NV_packed_depth_stencil

   #define GL_NV_packed_depth_stencil 1
   #define GL_NV_packed_depth_stencil_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DEPTH_STENCIL_NV

      #define GL_DEPTH_STENCIL_NV                                         0x84f9
      #define GL_UNSIGNED_INT_24_8_NV                                     0x84fa

   #endif /* GL_DEPTH_STENCIL_NV */

#endif /* GL_NV_packed_depth_stencil */


/* ---[ GL_NV_pixel_data_range ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/pixel_data_range.txt */

#ifndef GL_NV_pixel_data_range

   #define GL_NV_pixel_data_range 1
   #define GL_NV_pixel_data_range_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_WRITE_PIXEL_DATA_RANGE_NV

      #define GL_WRITE_PIXEL_DATA_RANGE_NV                                0x8878
      #define GL_READ_PIXEL_DATA_RANGE_NV                                 0x8879
      #define GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV                         0x887a
      #define GL_READ_PIXEL_DATA_RANGE_LENGTH_NV                          0x887b
      #define GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV                        0x887c
      #define GL_READ_PIXEL_DATA_RANGE_POINTER_NV                         0x887d

   #endif /* GL_WRITE_PIXEL_DATA_RANGE_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFlushPixelDataRangeNV                          OGLEXT_MAKEGLNAME(FlushPixelDataRangeNV)
      #define glPixelDataRangeNV                               OGLEXT_MAKEGLNAME(PixelDataRangeNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFlushPixelDataRangeNV(GLenum);
      GLAPI GLvoid            glPixelDataRangeNV(GLenum, GLsizei, GLvoid *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_pixel_data_range */


/* ---[ GL_NV_point_sprite ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/point_sprite.txt */

#ifndef GL_NV_point_sprite

   #define GL_NV_point_sprite 1
   #define GL_NV_point_sprite_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_POINT_SPRITE_NV

      #define GL_POINT_SPRITE_NV                                          0x8861
      #define GL_COORD_REPLACE_NV                                         0x8862
      #define GL_POINT_SPRITE_R_MODE_NV                                   0x8863

   #endif /* GL_POINT_SPRITE_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPointParameteriNV                              OGLEXT_MAKEGLNAME(PointParameteriNV)
      #define glPointParameterivNV                             OGLEXT_MAKEGLNAME(PointParameterivNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPointParameteriNV(GLenum, GLint);
      GLAPI GLvoid            glPointParameterivNV(GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_point_sprite */


/* ---[ GL_NV_primitive_restart ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/primitive_restart.txt */

#ifndef GL_NV_primitive_restart

   #define GL_NV_primitive_restart 1
   #define GL_NV_primitive_restart_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PRIMITIVE_RESTART_NV

      #define GL_PRIMITIVE_RESTART_NV                                     0x8558
      #define GL_PRIMITIVE_RESTART_INDEX_NV                               0x8559

   #endif /* GL_PRIMITIVE_RESTART_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPrimitiveRestartIndexNV                        OGLEXT_MAKEGLNAME(PrimitiveRestartIndexNV)
      #define glPrimitiveRestartNV                             OGLEXT_MAKEGLNAME(PrimitiveRestartNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPrimitiveRestartIndexNV(GLuint);
      GLAPI GLvoid            glPrimitiveRestartNV();

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_primitive_restart */


/* ---[ GL_NV_register_combiners ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/register_combiners.txt */

#ifndef GL_NV_register_combiners

   #define GL_NV_register_combiners 1
   #define GL_NV_register_combiners_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_REGISTER_COMBINERS_NV

      #define GL_REGISTER_COMBINERS_NV                                    0x8522
      #define GL_VARIABLE_A_NV                                            0x8523
      #define GL_VARIABLE_B_NV                                            0x8524
      #define GL_VARIABLE_C_NV                                            0x8525
      #define GL_VARIABLE_D_NV                                            0x8526
      #define GL_VARIABLE_E_NV                                            0x8527
      #define GL_VARIABLE_F_NV                                            0x8528
      #define GL_VARIABLE_G_NV                                            0x8529
      #define GL_CONSTANT_COLOR0_NV                                       0x852a
      #define GL_CONSTANT_COLOR1_NV                                       0x852b
      #define GL_PRIMARY_COLOR_NV                                         0x852c
      #define GL_SECONDARY_COLOR_NV                                       0x852d
      #define GL_SPARE0_NV                                                0x852e
      #define GL_SPARE1_NV                                                0x852f
      #define GL_DISCARD_NV                                               0x8530
      #define GL_E_TIMES_F_NV                                             0x8531
      #define GL_SPARE0_PLUS_SECONDARY_COLOR_NV                           0x8532
      #define GL_UNSIGNED_IDENTITY_NV                                     0x8536
      #define GL_UNSIGNED_INVERT_NV                                       0x8537
      #define GL_EXPAND_NORMAL_NV                                         0x8538
      #define GL_EXPAND_NEGATE_NV                                         0x8539
      #define GL_HALF_BIAS_NORMAL_NV                                      0x853a
      #define GL_HALF_BIAS_NEGATE_NV                                      0x853b
      #define GL_SIGNED_IDENTITY_NV                                       0x853c
      #define GL_SIGNED_NEGATE_NV                                         0x853d
      #define GL_SCALE_BY_TWO_NV                                          0x853e
      #define GL_SCALE_BY_FOUR_NV                                         0x853f
      #define GL_SCALE_BY_ONE_HALF_NV                                     0x8540
      #define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV                             0x8541
      #define GL_COMBINER_INPUT_NV                                        0x8542
      #define GL_COMBINER_MAPPING_NV                                      0x8543
      #define GL_COMBINER_COMPONENT_USAGE_NV                              0x8544
      #define GL_COMBINER_AB_DOT_PRODUCT_NV                               0x8545
      #define GL_COMBINER_CD_DOT_PRODUCT_NV                               0x8546
      #define GL_COMBINER_MUX_SUM_NV                                      0x8547
      #define GL_COMBINER_SCALE_NV                                        0x8548
      #define GL_COMBINER_BIAS_NV                                         0x8549
      #define GL_COMBINER_AB_OUTPUT_NV                                    0x854a
      #define GL_COMBINER_CD_OUTPUT_NV                                    0x854b
      #define GL_COMBINER_SUM_OUTPUT_NV                                   0x854c
      #define GL_MAX_GENERAL_COMBINERS_NV                                 0x854d
      #define GL_NUM_GENERAL_COMBINERS_NV                                 0x854e
      #define GL_COLOR_SUM_CLAMP_NV                                       0x854f
      #define GL_COMBINER0_NV                                             0x8550
      #define GL_COMBINER1_NV                                             0x8551
      #define GL_COMBINER2_NV                                             0x8552
      #define GL_COMBINER3_NV                                             0x8553
      #define GL_COMBINER4_NV                                             0x8554
      #define GL_COMBINER5_NV                                             0x8555
      #define GL_COMBINER6_NV                                             0x8556
      #define GL_COMBINER7_NV                                             0x8557

   #endif /* GL_REGISTER_COMBINERS_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glCombinerInputNV                                OGLEXT_MAKEGLNAME(CombinerInputNV)
      #define glCombinerOutputNV                               OGLEXT_MAKEGLNAME(CombinerOutputNV)
      #define glCombinerParameterfNV                           OGLEXT_MAKEGLNAME(CombinerParameterfNV)
      #define glCombinerParameterfvNV                          OGLEXT_MAKEGLNAME(CombinerParameterfvNV)
      #define glCombinerParameteriNV                           OGLEXT_MAKEGLNAME(CombinerParameteriNV)
      #define glCombinerParameterivNV                          OGLEXT_MAKEGLNAME(CombinerParameterivNV)
      #define glFinalCombinerInputNV                           OGLEXT_MAKEGLNAME(FinalCombinerInputNV)
      #define glGetCombinerInputParameterfvNV                  OGLEXT_MAKEGLNAME(GetCombinerInputParameterfvNV)
      #define glGetCombinerInputParameterivNV                  OGLEXT_MAKEGLNAME(GetCombinerInputParameterivNV)
      #define glGetCombinerOutputParameterfvNV                 OGLEXT_MAKEGLNAME(GetCombinerOutputParameterfvNV)
      #define glGetCombinerOutputParameterivNV                 OGLEXT_MAKEGLNAME(GetCombinerOutputParameterivNV)
      #define glGetFinalCombinerInputParameterfvNV             OGLEXT_MAKEGLNAME(GetFinalCombinerInputParameterfvNV)
      #define glGetFinalCombinerInputParameterivNV             OGLEXT_MAKEGLNAME(GetFinalCombinerInputParameterivNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glCombinerInputNV(GLenum, GLenum, GLenum, GLenum, GLenum, GLenum);
      GLAPI GLvoid            glCombinerOutputNV(GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLboolean, GLboolean, GLboolean);
      GLAPI GLvoid            glCombinerParameterfNV(GLenum, GLfloat);
      GLAPI GLvoid            glCombinerParameterfvNV(GLenum, GLfloat const *);
      GLAPI GLvoid            glCombinerParameteriNV(GLenum, GLint);
      GLAPI GLvoid            glCombinerParameterivNV(GLenum, GLint const *);
      GLAPI GLvoid            glFinalCombinerInputNV(GLenum, GLenum, GLenum, GLenum);
      GLAPI GLvoid            glGetCombinerInputParameterfvNV(GLenum, GLenum, GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetCombinerInputParameterivNV(GLenum, GLenum, GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetCombinerOutputParameterfvNV(GLenum, GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetCombinerOutputParameterivNV(GLenum, GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetFinalCombinerInputParameterfvNV(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetFinalCombinerInputParameterivNV(GLenum, GLenum, GLint *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_register_combiners */


/* ---[ GL_NV_register_combiners2 ]-------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/register_combiners2.txt */

#ifndef GL_NV_register_combiners2

   #define GL_NV_register_combiners2 1
   #define GL_NV_register_combiners2_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PER_STAGE_CONSTANTS_NV

      #define GL_PER_STAGE_CONSTANTS_NV                                   0x8535

   #endif /* GL_PER_STAGE_CONSTANTS_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glCombinerStageParameterfvNV                     OGLEXT_MAKEGLNAME(CombinerStageParameterfvNV)
      #define glGetCombinerStageParameterfvNV                  OGLEXT_MAKEGLNAME(GetCombinerStageParameterfvNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glCombinerStageParameterfvNV(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glGetCombinerStageParameterfvNV(GLenum, GLenum, GLfloat *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_register_combiners2 */


/* ---[ GL_NV_texgen_emboss ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texgen_emboss.txt */

#ifndef GL_NV_texgen_emboss

   #define GL_NV_texgen_emboss 1
   #define GL_NV_texgen_emboss_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_EMBOSS_LIGHT_NV

      #define GL_EMBOSS_LIGHT_NV                                          0x855d
      #define GL_EMBOSS_CONSTANT_NV                                       0x855e
      #define GL_EMBOSS_MAP_NV                                            0x855f

   #endif /* GL_EMBOSS_LIGHT_NV */

#endif /* GL_NV_texgen_emboss */


/* ---[ GL_NV_texgen_reflection ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texgen_reflection.txt */

#ifndef GL_NV_texgen_reflection

   #define GL_NV_texgen_reflection 1
   #define GL_NV_texgen_reflection_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_NORMAL_MAP_NV

      #define GL_NORMAL_MAP_NV                                            0x8511
      #define GL_REFLECTION_MAP_NV                                        0x8512

   #endif /* GL_NORMAL_MAP_NV */

#endif /* GL_NV_texgen_reflection */


/* ---[ GL_NV_texture_compression_vtc ]---------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_compression_vtc.txt */

#ifndef GL_NV_texture_compression_vtc

   #define GL_NV_texture_compression_vtc 1
   #define GL_NV_texture_compression_vtc_OGLEXT 1

#endif /* GL_NV_texture_compression_vtc */


/* ---[ GL_NV_texture_env_combine4 ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_env_combine4.txt */

#ifndef GL_NV_texture_env_combine4

   #define GL_NV_texture_env_combine4 1
   #define GL_NV_texture_env_combine4_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COMBINE4_NV

      #define GL_COMBINE4_NV                                              0x8503
      #define GL_SOURCE3_RGB_NV                                           0x8583
      #define GL_SOURCE3_ALPHA_NV                                         0x858b
      #define GL_OPERAND3_RGB_NV                                          0x8593
      #define GL_OPERAND3_ALPHA_NV                                        0x859b

   #endif /* GL_COMBINE4_NV */

#endif /* GL_NV_texture_env_combine4 */


/* ---[ GL_NV_texture_expand_normal ]------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_expand_normal.txt */

#ifndef GL_NV_texture_expand_normal

   #define GL_NV_texture_expand_normal 1
   #define GL_NV_texture_expand_normal_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_UNSIGNED_REMAP_MODE_NV

      #define GL_TEXTURE_UNSIGNED_REMAP_MODE_NV                           0x888f

   #endif /* GL_TEXTURE_UNSIGNED_REMAP_MODE_NV */

#endif /* GL_NV_texture_expand_normal */


/* ---[ GL_NV_texture_rectangle ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_rectangle.txt */

#ifndef GL_NV_texture_rectangle

   #define GL_NV_texture_rectangle 1
   #define GL_NV_texture_rectangle_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_RECTANGLE_NV

      #define GL_TEXTURE_RECTANGLE_NV                                     0x84f5
      #define GL_TEXTURE_BINDING_RECTANGLE_NV                             0x84f6
      #define GL_PROXY_TEXTURE_RECTANGLE_NV                               0x84f7
      #define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV                            0x84f8

   #endif /* GL_TEXTURE_RECTANGLE_NV */

#endif /* GL_NV_texture_rectangle */


/* ---[ GL_NV_texture_shader ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_shader.txt */

#ifndef GL_NV_texture_shader

   #define GL_NV_texture_shader 1
   #define GL_NV_texture_shader_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_OFFSET_TEXTURE_RECTANGLE_NV

      #define GL_OFFSET_TEXTURE_RECTANGLE_NV                              0x864c
      #define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV                        0x864d
      #define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV                         0x864e
      #define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV                     0x86d9
      #define GL_UNSIGNED_INT_S8_S8_8_8_NV                                0x86da
      #define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV                            0x86db
      #define GL_DSDT_MAG_INTENSITY_NV                                    0x86dc
      #define GL_SHADER_CONSISTENT_NV                                     0x86dd
      #define GL_TEXTURE_SHADER_NV                                        0x86de
      #define GL_SHADER_OPERATION_NV                                      0x86df
      #define GL_CULL_MODES_NV                                            0x86e0
      #define GL_OFFSET_TEXTURE_MATRIX_NV                                 0x86e1
      #define GL_OFFSET_TEXTURE_2D_MATRIX_NV                              0x86e1
      #define GL_OFFSET_TEXTURE_SCALE_NV                                  0x86e2
      #define GL_OFFSET_TEXTURE_2D_SCALE_NV                               0x86e2
      #define GL_OFFSET_TEXTURE_BIAS_NV                                   0x86e3
      #define GL_OFFSET_TEXTURE_2D_BIAS_NV                                0x86e3
      #define GL_PREVIOUS_TEXTURE_INPUT_NV                                0x86e4
      #define GL_CONST_EYE_NV                                             0x86e5
      #define GL_PASS_THROUGH_NV                                          0x86e6
      #define GL_CULL_FRAGMENT_NV                                         0x86e7
      #define GL_OFFSET_TEXTURE_2D_NV                                     0x86e8
      #define GL_DEPENDENT_AR_TEXTURE_2D_NV                               0x86e9
      #define GL_DEPENDENT_GB_TEXTURE_2D_NV                               0x86ea
      #define GL_DOT_PRODUCT_NV                                           0x86ec
      #define GL_DOT_PRODUCT_DEPTH_REPLACE_NV                             0x86ed
      #define GL_DOT_PRODUCT_TEXTURE_2D_NV                                0x86ee
      #define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV                          0x86f0
      #define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV                          0x86f1
      #define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV                          0x86f2
      #define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV                0x86f3
      #define GL_HILO_NV                                                  0x86f4
      #define GL_DSDT_NV                                                  0x86f5
      #define GL_DSDT_MAG_NV                                              0x86f6
      #define GL_DSDT_MAG_VIB_NV                                          0x86f7
      #define GL_HILO16_NV                                                0x86f8
      #define GL_SIGNED_HILO_NV                                           0x86f9
      #define GL_SIGNED_HILO16_NV                                         0x86fa
      #define GL_SIGNED_RGBA_NV                                           0x86fb
      #define GL_SIGNED_RGBA8_NV                                          0x86fc
      #define GL_SIGNED_RGB_NV                                            0x86fe
      #define GL_SIGNED_RGB8_NV                                           0x86ff
      #define GL_SIGNED_LUMINANCE_NV                                      0x8701
      #define GL_SIGNED_LUMINANCE8_NV                                     0x8702
      #define GL_SIGNED_LUMINANCE_ALPHA_NV                                0x8703
      #define GL_SIGNED_LUMINANCE8_ALPHA8_NV                              0x8704
      #define GL_SIGNED_ALPHA_NV                                          0x8705
      #define GL_SIGNED_ALPHA8_NV                                         0x8706
      #define GL_SIGNED_INTENSITY_NV                                      0x8707
      #define GL_SIGNED_INTENSITY8_NV                                     0x8708
      #define GL_DSDT8_NV                                                 0x8709
      #define GL_DSDT8_MAG8_NV                                            0x870a
      #define GL_DSDT8_MAG8_INTENSITY8_NV                                 0x870b
      #define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV                             0x870c
      #define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV                           0x870d
      #define GL_HI_SCALE_NV                                              0x870e
      #define GL_LO_SCALE_NV                                              0x870f
      #define GL_DS_SCALE_NV                                              0x8710
      #define GL_DT_SCALE_NV                                              0x8711
      #define GL_MAGNITUDE_SCALE_NV                                       0x8712
      #define GL_VIBRANCE_SCALE_NV                                        0x8713
      #define GL_HI_BIAS_NV                                               0x8714
      #define GL_LO_BIAS_NV                                               0x8715
      #define GL_DS_BIAS_NV                                               0x8716
      #define GL_DT_BIAS_NV                                               0x8717
      #define GL_MAGNITUDE_BIAS_NV                                        0x8718
      #define GL_VIBRANCE_BIAS_NV                                         0x8719
      #define GL_TEXTURE_BORDER_VALUES_NV                                 0x871a
      #define GL_TEXTURE_HI_SIZE_NV                                       0x871b
      #define GL_TEXTURE_LO_SIZE_NV                                       0x871c
      #define GL_TEXTURE_DS_SIZE_NV                                       0x871d
      #define GL_TEXTURE_DT_SIZE_NV                                       0x871e
      #define GL_TEXTURE_MAG_SIZE_NV                                      0x871f

   #endif /* GL_OFFSET_TEXTURE_RECTANGLE_NV */

#endif /* GL_NV_texture_shader */


/* ---[ GL_NV_texture_shader2 ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_shader2.txt */

#ifndef GL_NV_texture_shader2

   #define GL_NV_texture_shader2 1
   #define GL_NV_texture_shader2_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DOT_PRODUCT_TEXTURE_3D_NV

      #define GL_DOT_PRODUCT_TEXTURE_3D_NV                                0x86ef

   #endif /* GL_DOT_PRODUCT_TEXTURE_3D_NV */

#endif /* GL_NV_texture_shader2 */


/* ---[ GL_NV_texture_shader3 ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/texture_shader3.txt */

#ifndef GL_NV_texture_shader3

   #define GL_NV_texture_shader3 1
   #define GL_NV_texture_shader3_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV

      #define GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV                          0x8850
      #define GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV                    0x8851
      #define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV                   0x8852
      #define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV             0x8853
      #define GL_OFFSET_HILO_TEXTURE_2D_NV                                0x8854
      #define GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV                         0x8855
      #define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV                     0x8856
      #define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV              0x8857
      #define GL_DEPENDENT_HILO_TEXTURE_2D_NV                             0x8858
      #define GL_DEPENDENT_RGB_TEXTURE_3D_NV                              0x8859
      #define GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV                        0x885a
      #define GL_DOT_PRODUCT_PASS_THROUGH_NV                              0x885b
      #define GL_DOT_PRODUCT_TEXTURE_1D_NV                                0x885c
      #define GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV                      0x885d
      #define GL_HILO8_NV                                                 0x885e
      #define GL_SIGNED_HILO8_NV                                          0x885f
      #define GL_FORCE_BLUE_TO_ONE_NV                                     0x8860

   #endif /* GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV */

#endif /* GL_NV_texture_shader3 */


/* ---[ GL_NV_vertex_array_range ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/vertex_array_range.txt */

#ifndef GL_NV_vertex_array_range

   #define GL_NV_vertex_array_range 1
   #define GL_NV_vertex_array_range_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_ARRAY_RANGE_NV

      #define GL_VERTEX_ARRAY_RANGE_NV                                    0x851d
      #define GL_VERTEX_ARRAY_RANGE_LENGTH_NV                             0x851e
      #define GL_VERTEX_ARRAY_RANGE_VALID_NV                              0x851f
      #define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV                        0x8520
      #define GL_VERTEX_ARRAY_RANGE_POINTER_NV                            0x8521

   #endif /* GL_VERTEX_ARRAY_RANGE_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFlushVertexArrayRangeNV                        OGLEXT_MAKEGLNAME(FlushVertexArrayRangeNV)
      #define glVertexArrayRangeNV                             OGLEXT_MAKEGLNAME(VertexArrayRangeNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFlushVertexArrayRangeNV();
      GLAPI GLvoid            glVertexArrayRangeNV(GLsizei, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_vertex_array_range */


/* ---[ GL_NV_vertex_array_range2 ]-------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/vertex_array_range2.txt */

#ifndef GL_NV_vertex_array_range2

   #define GL_NV_vertex_array_range2 1
   #define GL_NV_vertex_array_range2_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV

      #define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV                      0x8533

   #endif /* GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV */

#endif /* GL_NV_vertex_array_range2 */


/* ---[ GL_NV_vertex_program ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/vertex_program.txt */

#ifndef GL_NV_vertex_program

   #define GL_NV_vertex_program 1
   #define GL_NV_vertex_program_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_PROGRAM_NV

      #define GL_VERTEX_PROGRAM_NV                                        0x8620
      #define GL_VERTEX_STATE_PROGRAM_NV                                  0x8621
      #define GL_ATTRIB_ARRAY_SIZE_NV                                     0x8623
      #define GL_ATTRIB_ARRAY_STRIDE_NV                                   0x8624
      #define GL_ATTRIB_ARRAY_TYPE_NV                                     0x8625
      #define GL_CURRENT_ATTRIB_NV                                        0x8626
      #define GL_PROGRAM_LENGTH_NV                                        0x8627
      #define GL_PROGRAM_STRING_NV                                        0x8628
      #define GL_MODELVIEW_PROJECTION_NV                                  0x8629
      #define GL_IDENTITY_NV                                              0x862a
      #define GL_INVERSE_NV                                               0x862b
      #define GL_TRANSPOSE_NV                                             0x862c
      #define GL_INVERSE_TRANSPOSE_NV                                     0x862d
      #define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV                          0x862e
      #define GL_MAX_TRACK_MATRICES_NV                                    0x862f
      #define GL_MATRIX0_NV                                               0x8630
      #define GL_MATRIX1_NV                                               0x8631
      #define GL_MATRIX2_NV                                               0x8632
      #define GL_MATRIX3_NV                                               0x8633
      #define GL_MATRIX4_NV                                               0x8634
      #define GL_MATRIX5_NV                                               0x8635
      #define GL_MATRIX6_NV                                               0x8636
      #define GL_MATRIX7_NV                                               0x8637
      #define GL_CURRENT_MATRIX_STACK_DEPTH_NV                            0x8640
      #define GL_CURRENT_MATRIX_NV                                        0x8641
      #define GL_VERTEX_PROGRAM_POINT_SIZE_NV                             0x8642
      #define GL_VERTEX_PROGRAM_TWO_SIDE_NV                               0x8643
      #define GL_PROGRAM_PARAMETER_NV                                     0x8644
      #define GL_ATTRIB_ARRAY_POINTER_NV                                  0x8645
      #define GL_PROGRAM_TARGET_NV                                        0x8646
      #define GL_PROGRAM_RESIDENT_NV                                      0x8647
      #define GL_TRACK_MATRIX_NV                                          0x8648
      #define GL_TRACK_MATRIX_TRANSFORM_NV                                0x8649
      #define GL_VERTEX_PROGRAM_BINDING_NV                                0x864a
      #define GL_PROGRAM_ERROR_POSITION_NV                                0x864b
      #define GL_VERTEX_ATTRIB_ARRAY0_NV                                  0x8650
      #define GL_VERTEX_ATTRIB_ARRAY1_NV                                  0x8651
      #define GL_VERTEX_ATTRIB_ARRAY2_NV                                  0x8652
      #define GL_VERTEX_ATTRIB_ARRAY3_NV                                  0x8653
      #define GL_VERTEX_ATTRIB_ARRAY4_NV                                  0x8654
      #define GL_VERTEX_ATTRIB_ARRAY5_NV                                  0x8655
      #define GL_VERTEX_ATTRIB_ARRAY6_NV                                  0x8656
      #define GL_VERTEX_ATTRIB_ARRAY7_NV                                  0x8657
      #define GL_VERTEX_ATTRIB_ARRAY8_NV                                  0x8658
      #define GL_VERTEX_ATTRIB_ARRAY9_NV                                  0x8659
      #define GL_VERTEX_ATTRIB_ARRAY10_NV                                 0x865a
      #define GL_VERTEX_ATTRIB_ARRAY11_NV                                 0x865b
      #define GL_VERTEX_ATTRIB_ARRAY12_NV                                 0x865c
      #define GL_VERTEX_ATTRIB_ARRAY13_NV                                 0x865d
      #define GL_VERTEX_ATTRIB_ARRAY14_NV                                 0x865e
      #define GL_VERTEX_ATTRIB_ARRAY15_NV                                 0x865f
      #define GL_MAP1_VERTEX_ATTRIB0_4_NV                                 0x8660
      #define GL_MAP1_VERTEX_ATTRIB1_4_NV                                 0x8661
      #define GL_MAP1_VERTEX_ATTRIB2_4_NV                                 0x8662
      #define GL_MAP1_VERTEX_ATTRIB3_4_NV                                 0x8663
      #define GL_MAP1_VERTEX_ATTRIB4_4_NV                                 0x8664
      #define GL_MAP1_VERTEX_ATTRIB5_4_NV                                 0x8665
      #define GL_MAP1_VERTEX_ATTRIB6_4_NV                                 0x8666
      #define GL_MAP1_VERTEX_ATTRIB7_4_NV                                 0x8667
      #define GL_MAP1_VERTEX_ATTRIB8_4_NV                                 0x8668
      #define GL_MAP1_VERTEX_ATTRIB9_4_NV                                 0x8669
      #define GL_MAP1_VERTEX_ATTRIB10_4_NV                                0x866a
      #define GL_MAP1_VERTEX_ATTRIB11_4_NV                                0x866b
      #define GL_MAP1_VERTEX_ATTRIB12_4_NV                                0x866c
      #define GL_MAP1_VERTEX_ATTRIB13_4_NV                                0x866d
      #define GL_MAP1_VERTEX_ATTRIB14_4_NV                                0x866e
      #define GL_MAP1_VERTEX_ATTRIB15_4_NV                                0x866f
      #define GL_MAP2_VERTEX_ATTRIB0_4_NV                                 0x8670
      #define GL_MAP2_VERTEX_ATTRIB1_4_NV                                 0x8671
      #define GL_MAP2_VERTEX_ATTRIB2_4_NV                                 0x8672
      #define GL_MAP2_VERTEX_ATTRIB3_4_NV                                 0x8673
      #define GL_MAP2_VERTEX_ATTRIB4_4_NV                                 0x8674
      #define GL_MAP2_VERTEX_ATTRIB5_4_NV                                 0x8675
      #define GL_MAP2_VERTEX_ATTRIB6_4_NV                                 0x8676
      #define GL_MAP2_VERTEX_ATTRIB7_4_NV                                 0x8677
      #define GL_MAP2_VERTEX_ATTRIB8_4_NV                                 0x8678
      #define GL_MAP2_VERTEX_ATTRIB9_4_NV                                 0x8679
      #define GL_MAP2_VERTEX_ATTRIB10_4_NV                                0x867a
      #define GL_MAP2_VERTEX_ATTRIB11_4_NV                                0x867b
      #define GL_MAP2_VERTEX_ATTRIB12_4_NV                                0x867c
      #define GL_MAP2_VERTEX_ATTRIB13_4_NV                                0x867d
      #define GL_MAP2_VERTEX_ATTRIB14_4_NV                                0x867e
      #define GL_MAP2_VERTEX_ATTRIB15_4_NV                                0x867f

   #endif /* GL_VERTEX_PROGRAM_NV */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glAreProgramsResidentNV                          OGLEXT_MAKEGLNAME(AreProgramsResidentNV)
      #define glBindProgramNV                                  OGLEXT_MAKEGLNAME(BindProgramNV)
      #define glDeleteProgramsNV                               OGLEXT_MAKEGLNAME(DeleteProgramsNV)
      #define glExecuteProgramNV                               OGLEXT_MAKEGLNAME(ExecuteProgramNV)
      #define glGenProgramsNV                                  OGLEXT_MAKEGLNAME(GenProgramsNV)
      #define glGetProgramivNV                                 OGLEXT_MAKEGLNAME(GetProgramivNV)
      #define glGetProgramParameterdvNV                        OGLEXT_MAKEGLNAME(GetProgramParameterdvNV)
      #define glGetProgramParameterfvNV                        OGLEXT_MAKEGLNAME(GetProgramParameterfvNV)
      #define glGetProgramStringNV                             OGLEXT_MAKEGLNAME(GetProgramStringNV)
      #define glGetTrackMatrixivNV                             OGLEXT_MAKEGLNAME(GetTrackMatrixivNV)
      #define glGetVertexAttribdvNV                            OGLEXT_MAKEGLNAME(GetVertexAttribdvNV)
      #define glGetVertexAttribfvNV                            OGLEXT_MAKEGLNAME(GetVertexAttribfvNV)
      #define glGetVertexAttribivNV                            OGLEXT_MAKEGLNAME(GetVertexAttribivNV)
      #define glGetVertexAttribPointervNV                      OGLEXT_MAKEGLNAME(GetVertexAttribPointervNV)
      #define glIsProgramNV                                    OGLEXT_MAKEGLNAME(IsProgramNV)
      #define glLoadProgramNV                                  OGLEXT_MAKEGLNAME(LoadProgramNV)
      #define glProgramParameter4dNV                           OGLEXT_MAKEGLNAME(ProgramParameter4dNV)
      #define glProgramParameter4dvNV                          OGLEXT_MAKEGLNAME(ProgramParameter4dvNV)
      #define glProgramParameter4fNV                           OGLEXT_MAKEGLNAME(ProgramParameter4fNV)
      #define glProgramParameter4fvNV                          OGLEXT_MAKEGLNAME(ProgramParameter4fvNV)
      #define glProgramParameters4dvNV                         OGLEXT_MAKEGLNAME(ProgramParameters4dvNV)
      #define glProgramParameters4fvNV                         OGLEXT_MAKEGLNAME(ProgramParameters4fvNV)
      #define glRequestResidentProgramsNV                      OGLEXT_MAKEGLNAME(RequestResidentProgramsNV)
      #define glTrackMatrixNV                                  OGLEXT_MAKEGLNAME(TrackMatrixNV)
      #define glVertexAttrib1dNV                               OGLEXT_MAKEGLNAME(VertexAttrib1dNV)
      #define glVertexAttrib1dvNV                              OGLEXT_MAKEGLNAME(VertexAttrib1dvNV)
      #define glVertexAttrib1fNV                               OGLEXT_MAKEGLNAME(VertexAttrib1fNV)
      #define glVertexAttrib1fvNV                              OGLEXT_MAKEGLNAME(VertexAttrib1fvNV)
      #define glVertexAttrib1sNV                               OGLEXT_MAKEGLNAME(VertexAttrib1sNV)
      #define glVertexAttrib1svNV                              OGLEXT_MAKEGLNAME(VertexAttrib1svNV)
      #define glVertexAttrib2dNV                               OGLEXT_MAKEGLNAME(VertexAttrib2dNV)
      #define glVertexAttrib2dvNV                              OGLEXT_MAKEGLNAME(VertexAttrib2dvNV)
      #define glVertexAttrib2fNV                               OGLEXT_MAKEGLNAME(VertexAttrib2fNV)
      #define glVertexAttrib2fvNV                              OGLEXT_MAKEGLNAME(VertexAttrib2fvNV)
      #define glVertexAttrib2sNV                               OGLEXT_MAKEGLNAME(VertexAttrib2sNV)
      #define glVertexAttrib2svNV                              OGLEXT_MAKEGLNAME(VertexAttrib2svNV)
      #define glVertexAttrib3dNV                               OGLEXT_MAKEGLNAME(VertexAttrib3dNV)
      #define glVertexAttrib3dvNV                              OGLEXT_MAKEGLNAME(VertexAttrib3dvNV)
      #define glVertexAttrib3fNV                               OGLEXT_MAKEGLNAME(VertexAttrib3fNV)
      #define glVertexAttrib3fvNV                              OGLEXT_MAKEGLNAME(VertexAttrib3fvNV)
      #define glVertexAttrib3sNV                               OGLEXT_MAKEGLNAME(VertexAttrib3sNV)
      #define glVertexAttrib3svNV                              OGLEXT_MAKEGLNAME(VertexAttrib3svNV)
      #define glVertexAttrib4dNV                               OGLEXT_MAKEGLNAME(VertexAttrib4dNV)
      #define glVertexAttrib4dvNV                              OGLEXT_MAKEGLNAME(VertexAttrib4dvNV)
      #define glVertexAttrib4fNV                               OGLEXT_MAKEGLNAME(VertexAttrib4fNV)
      #define glVertexAttrib4fvNV                              OGLEXT_MAKEGLNAME(VertexAttrib4fvNV)
      #define glVertexAttrib4sNV                               OGLEXT_MAKEGLNAME(VertexAttrib4sNV)
      #define glVertexAttrib4svNV                              OGLEXT_MAKEGLNAME(VertexAttrib4svNV)
      #define glVertexAttrib4ubNV                              OGLEXT_MAKEGLNAME(VertexAttrib4ubNV)
      #define glVertexAttrib4ubvNV                             OGLEXT_MAKEGLNAME(VertexAttrib4ubvNV)
      #define glVertexAttribPointerNV                          OGLEXT_MAKEGLNAME(VertexAttribPointerNV)
      #define glVertexAttribs1dvNV                             OGLEXT_MAKEGLNAME(VertexAttribs1dvNV)
      #define glVertexAttribs1fvNV                             OGLEXT_MAKEGLNAME(VertexAttribs1fvNV)
      #define glVertexAttribs1svNV                             OGLEXT_MAKEGLNAME(VertexAttribs1svNV)
      #define glVertexAttribs2dvNV                             OGLEXT_MAKEGLNAME(VertexAttribs2dvNV)
      #define glVertexAttribs2fvNV                             OGLEXT_MAKEGLNAME(VertexAttribs2fvNV)
      #define glVertexAttribs2svNV                             OGLEXT_MAKEGLNAME(VertexAttribs2svNV)
      #define glVertexAttribs3dvNV                             OGLEXT_MAKEGLNAME(VertexAttribs3dvNV)
      #define glVertexAttribs3fvNV                             OGLEXT_MAKEGLNAME(VertexAttribs3fvNV)
      #define glVertexAttribs3svNV                             OGLEXT_MAKEGLNAME(VertexAttribs3svNV)
      #define glVertexAttribs4dvNV                             OGLEXT_MAKEGLNAME(VertexAttribs4dvNV)
      #define glVertexAttribs4fvNV                             OGLEXT_MAKEGLNAME(VertexAttribs4fvNV)
      #define glVertexAttribs4svNV                             OGLEXT_MAKEGLNAME(VertexAttribs4svNV)
      #define glVertexAttribs4ubvNV                            OGLEXT_MAKEGLNAME(VertexAttribs4ubvNV)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLboolean         glAreProgramsResidentNV(GLsizei, GLuint const *, GLboolean *);
      GLAPI GLvoid            glBindProgramNV(GLenum, GLuint);
      GLAPI GLvoid            glDeleteProgramsNV(GLsizei, GLuint const *);
      GLAPI GLvoid            glExecuteProgramNV(GLenum, GLuint, GLfloat const *);
      GLAPI GLvoid            glGenProgramsNV(GLsizei, GLuint *);
      GLAPI GLvoid            glGetProgramivNV(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetProgramParameterdvNV(GLenum, GLuint, GLenum, GLdouble *);
      GLAPI GLvoid            glGetProgramParameterfvNV(GLenum, GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetProgramStringNV(GLuint, GLenum, GLubyte *);
      GLAPI GLvoid            glGetTrackMatrixivNV(GLenum, GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetVertexAttribdvNV(GLuint, GLenum, GLdouble *);
      GLAPI GLvoid            glGetVertexAttribfvNV(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetVertexAttribivNV(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glGetVertexAttribPointervNV(GLuint, GLenum, GLvoid * *);
      GLAPI GLboolean         glIsProgramNV(GLuint);
      GLAPI GLvoid            glLoadProgramNV(GLenum, GLuint, GLsizei, GLubyte const *);
      GLAPI GLvoid            glProgramParameter4dNV(GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glProgramParameter4dvNV(GLenum, GLuint, GLdouble const *);
      GLAPI GLvoid            glProgramParameter4fNV(GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glProgramParameter4fvNV(GLenum, GLuint, GLfloat const *);
      GLAPI GLvoid            glProgramParameters4dvNV(GLenum, GLuint, GLuint, GLdouble const *);
      GLAPI GLvoid            glProgramParameters4fvNV(GLenum, GLuint, GLuint, GLfloat const *);
      GLAPI GLvoid            glRequestResidentProgramsNV(GLsizei, GLuint const *);
      GLAPI GLvoid            glTrackMatrixNV(GLenum, GLuint, GLenum, GLenum);
      GLAPI GLvoid            glVertexAttrib1dNV(GLuint, GLdouble);
      GLAPI GLvoid            glVertexAttrib1dvNV(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib1fNV(GLuint, GLfloat);
      GLAPI GLvoid            glVertexAttrib1fvNV(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib1sNV(GLuint, GLshort);
      GLAPI GLvoid            glVertexAttrib1svNV(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib2dNV(GLuint, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexAttrib2dvNV(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib2fNV(GLuint, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexAttrib2fvNV(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib2sNV(GLuint, GLshort, GLshort);
      GLAPI GLvoid            glVertexAttrib2svNV(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib3dNV(GLuint, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexAttrib3dvNV(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib3fNV(GLuint, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexAttrib3fvNV(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib3sNV(GLuint, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glVertexAttrib3svNV(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib4dNV(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glVertexAttrib4dvNV(GLuint, GLdouble const *);
      GLAPI GLvoid            glVertexAttrib4fNV(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glVertexAttrib4fvNV(GLuint, GLfloat const *);
      GLAPI GLvoid            glVertexAttrib4sNV(GLuint, GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glVertexAttrib4svNV(GLuint, GLshort const *);
      GLAPI GLvoid            glVertexAttrib4ubNV(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
      GLAPI GLvoid            glVertexAttrib4ubvNV(GLuint, GLubyte const *);
      GLAPI GLvoid            glVertexAttribPointerNV(GLuint, GLint, GLenum, GLsizei, GLvoid const *);
      GLAPI GLvoid            glVertexAttribs1dvNV(GLuint, GLsizei, GLdouble const *);
      GLAPI GLvoid            glVertexAttribs1fvNV(GLuint, GLsizei, GLfloat const *);
      GLAPI GLvoid            glVertexAttribs1svNV(GLuint, GLsizei, GLshort const *);
      GLAPI GLvoid            glVertexAttribs2dvNV(GLuint, GLsizei, GLdouble const *);
      GLAPI GLvoid            glVertexAttribs2fvNV(GLuint, GLsizei, GLfloat const *);
      GLAPI GLvoid            glVertexAttribs2svNV(GLuint, GLsizei, GLshort const *);
      GLAPI GLvoid            glVertexAttribs3dvNV(GLuint, GLsizei, GLdouble const *);
      GLAPI GLvoid            glVertexAttribs3fvNV(GLuint, GLsizei, GLfloat const *);
      GLAPI GLvoid            glVertexAttribs3svNV(GLuint, GLsizei, GLshort const *);
      GLAPI GLvoid            glVertexAttribs4dvNV(GLuint, GLsizei, GLdouble const *);
      GLAPI GLvoid            glVertexAttribs4fvNV(GLuint, GLsizei, GLfloat const *);
      GLAPI GLvoid            glVertexAttribs4svNV(GLuint, GLsizei, GLshort const *);
      GLAPI GLvoid            glVertexAttribs4ubvNV(GLuint, GLsizei, GLubyte const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_NV_vertex_program */


/* ---[ GL_NV_vertex_program1_1 ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/vertex_program1_1.txt */

#ifndef GL_NV_vertex_program1_1

   #define GL_NV_vertex_program1_1 1
   #define GL_NV_vertex_program1_1_OGLEXT 1

#endif /* GL_NV_vertex_program1_1 */


/* ---[ GL_NV_vertex_program2 ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/NV/vertex_program2.txt */

#ifndef GL_NV_vertex_program2

   #define GL_NV_vertex_program2 1
   #define GL_NV_vertex_program2_OGLEXT 1

#endif /* GL_NV_vertex_program2 */


/* ---[ GL_OML_interlace ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/OML/interlace.txt */

#ifndef GL_OML_interlace

   #define GL_OML_interlace 1
   #define GL_OML_interlace_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_INTERLACE_OML

      #define GL_INTERLACE_OML                                            0x8980
      #define GL_INTERLACE_READ_OML                                       0x8981

   #endif /* GL_INTERLACE_OML */

#endif /* GL_OML_interlace */


/* ---[ GL_OML_resample ]------------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/OML/resample.txt */

#ifndef GL_OML_resample

   #define GL_OML_resample 1
   #define GL_OML_resample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PACK_RESAMPLE_OML

      #define GL_PACK_RESAMPLE_OML                                        0x8984
      #define GL_UNPACK_RESAMPLE_OML                                      0x8985
      #define GL_RESAMPLE_REPLICATE_OML                                   0x8986
      #define GL_RESAMPLE_ZERO_FILL_OML                                   0x8987
      #define GL_RESAMPLE_AVERAGE_OML                                     0x8988
      #define GL_RESAMPLE_DECIMATE_OML                                    0x8989

   #endif /* GL_PACK_RESAMPLE_OML */

#endif /* GL_OML_resample */


/* ---[ GL_OML_subsample ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/OML/subsample.txt */

#ifndef GL_OML_subsample

   #define GL_OML_subsample 1
   #define GL_OML_subsample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FORMAT_SUBSAMPLE_24_24_OML

      #define GL_FORMAT_SUBSAMPLE_24_24_OML                               0x8982
      #define GL_FORMAT_SUBSAMPLE_244_244_OML                             0x8983

   #endif /* GL_FORMAT_SUBSAMPLE_24_24_OML */

#endif /* GL_OML_subsample */


/* ---[ GL_PGI_misc_hints ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/PGI/misc_hints.txt */

#ifndef GL_PGI_misc_hints

   #define GL_PGI_misc_hints 1
   #define GL_PGI_misc_hints_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PREFER_DOUBLEBUFFER_HINT_PGI

      #define GL_PREFER_DOUBLEBUFFER_HINT_PGI                             0x1a1f8
      #define GL_CONSERVE_MEMORY_HINT_PGI                                 0x1a1fd
      #define GL_RECLAIM_MEMORY_HINT_PGI                                  0x1a1fe
      #define GL_NATIVE_GRAPHICS_HANDLE_PGI                               0x1a202
      #define GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI                           0x1a203
      #define GL_NATIVE_GRAPHICS_END_HINT_PGI                             0x1a204
      #define GL_ALWAYS_FAST_HINT_PGI                                     0x1a20c
      #define GL_ALWAYS_SOFT_HINT_PGI                                     0x1a20d
      #define GL_ALLOW_DRAW_OBJ_HINT_PGI                                  0x1a20e
      #define GL_ALLOW_DRAW_WIN_HINT_PGI                                  0x1a20f
      #define GL_ALLOW_DRAW_FRG_HINT_PGI                                  0x1a210
      #define GL_ALLOW_DRAW_MEM_HINT_PGI                                  0x1a211
      #define GL_STRICT_DEPTHFUNC_HINT_PGI                                0x1a216
      #define GL_STRICT_LIGHTING_HINT_PGI                                 0x1a217
      #define GL_STRICT_SCISSOR_HINT_PGI                                  0x1a218
      #define GL_FULL_STIPPLE_HINT_PGI                                    0x1a219
      #define GL_CLIP_NEAR_HINT_PGI                                       0x1a220
      #define GL_CLIP_FAR_HINT_PGI                                        0x1a221
      #define GL_WIDE_LINE_HINT_PGI                                       0x1a222
      #define GL_BACK_NORMALS_HINT_PGI                                    0x1a223

   #endif /* GL_PREFER_DOUBLEBUFFER_HINT_PGI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glHintPGI                                        OGLEXT_MAKEGLNAME(HintPGI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glHintPGI(GLenum, GLint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_PGI_misc_hints */


/* ---[ GL_PGI_vertex_hints ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/PGI/vertex_hints.txt */

#ifndef GL_PGI_vertex_hints

   #define GL_PGI_vertex_hints 1
   #define GL_PGI_vertex_hints_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX23_BIT_PGI

      #define GL_VERTEX23_BIT_PGI                                         0x4
      #define GL_VERTEX4_BIT_PGI                                          0x8
      #define GL_COLOR3_BIT_PGI                                           0x10000
      #define GL_VERTEX_DATA_HINT_PGI                                     0x1a22a
      #define GL_VERTEX_CONSISTENT_HINT_PGI                               0x1a22b
      #define GL_MATERIAL_SIDE_HINT_PGI                                   0x1a22c
      #define GL_MAX_VERTEX_HINT_PGI                                      0x1a22d
      #define GL_COLOR4_BIT_PGI                                           0x20000
      #define GL_EDGEFLAG_BIT_PGI                                         0x40000
      #define GL_INDEX_BIT_PGI                                            0x80000
      #define GL_MAT_AMBIENT_BIT_PGI                                      0x100000
      #define GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI                          0x200000
      #define GL_MAT_DIFFUSE_BIT_PGI                                      0x400000
      #define GL_MAT_EMISSION_BIT_PGI                                     0x800000
      #define GL_MAT_COLOR_INDEXES_BIT_PGI                                0x1000000
      #define GL_MAT_SHININESS_BIT_PGI                                    0x2000000
      #define GL_MAT_SPECULAR_BIT_PGI                                     0x4000000
      #define GL_NORMAL_BIT_PGI                                           0x8000000
      #define GL_TEXCOORD1_BIT_PGI                                        0x10000000
      #define GL_TEXCOORD2_BIT_PGI                                        0x20000000
      #define GL_TEXCOORD3_BIT_PGI                                        0x40000000
      #define GL_TEXCOORD4_BIT_PGI                                        0x7fffffff

   #endif /* GL_VERTEX23_BIT_PGI */

#endif /* GL_PGI_vertex_hints */


/* ---[ GL_REND_screen_coordinates ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/REND/screen_coordinates.txt */

#ifndef GL_REND_screen_coordinates

   #define GL_REND_screen_coordinates 1
   #define GL_REND_screen_coordinates_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SCREEN_COORDINATES_REND

      #define GL_SCREEN_COORDINATES_REND                                  0x8490
      #define GL_INVERTED_SCREEN_W_REND                                   0x8491

   #endif /* GL_SCREEN_COORDINATES_REND */

#endif /* GL_REND_screen_coordinates */


/* ---[ GL_S3_s3tc ]----------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/S3/s3tc.txt */

#ifndef GL_S3_s3tc

   #define GL_S3_s3tc 1
   #define GL_S3_s3tc_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_RGB_S3TC

      #define GL_RGB_S3TC                                                 0x83a0
      #define GL_RGB4_S3TC                                                0x83a1
      #define GL_RGBA_S3TC                                                0x83a2
      #define GL_RGBA4_S3TC                                               0x83a3

   #endif /* GL_RGB_S3TC */

#endif /* GL_S3_s3tc */


/* ---[ GL_SGIS_detail_texture ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/detail_texture.txt */

#ifndef GL_SGIS_detail_texture

   #define GL_SGIS_detail_texture 1
   #define GL_SGIS_detail_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DETAIL_TEXTURE_2D_SGIS

      #define GL_DETAIL_TEXTURE_2D_SGIS                                   0x8095
      #define GL_DETAIL_TEXTURE_2D_BINDING_SGIS                           0x8096
      #define GL_LINEAR_DETAIL_SGIS                                       0x8097
      #define GL_LINEAR_DETAIL_ALPHA_SGIS                                 0x8098
      #define GL_LINEAR_DETAIL_COLOR_SGIS                                 0x8099
      #define GL_DETAIL_TEXTURE_LEVEL_SGIS                                0x809a
      #define GL_DETAIL_TEXTURE_MODE_SGIS                                 0x809b
      #define GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS                          0x809c

   #endif /* GL_DETAIL_TEXTURE_2D_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDetailTexFuncSGIS                              OGLEXT_MAKEGLNAME(DetailTexFuncSGIS)
      #define glGetDetailTexFuncSGIS                           OGLEXT_MAKEGLNAME(GetDetailTexFuncSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDetailTexFuncSGIS(GLenum, GLsizei, GLfloat const *);
      GLAPI GLvoid            glGetDetailTexFuncSGIS(GLenum, GLfloat *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_detail_texture */


/* ---[ GL_SGIS_fog_function ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/fog_func.txt */

#ifndef GL_SGIS_fog_function

   #define GL_SGIS_fog_function 1
   #define GL_SGIS_fog_function_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FOG_FUNC_SGIS

      #define GL_FOG_FUNC_SGIS                                            0x812a
      #define GL_FOG_FUNC_POINTS_SGIS                                     0x812b
      #define GL_MAX_FOG_FUNC_POINTS_SGIS                                 0x812c

   #endif /* GL_FOG_FUNC_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFogFuncSGIS                                    OGLEXT_MAKEGLNAME(FogFuncSGIS)
      #define glGetFogFuncSGIS                                 OGLEXT_MAKEGLNAME(GetFogFuncSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFogFuncSGIS(GLsizei, GLfloat const *);
      GLAPI GLvoid            glGetFogFuncSGIS(GLfloat *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_fog_function */


/* ---[ GL_SGIS_generate_mipmap ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/generate_mipmap.txt */

#ifndef GL_SGIS_generate_mipmap

   #define GL_SGIS_generate_mipmap 1
   #define GL_SGIS_generate_mipmap_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_GENERATE_MIPMAP_SGIS

      #define GL_GENERATE_MIPMAP_SGIS                                     0x8191
      #define GL_GENERATE_MIPMAP_HINT_SGIS                                0x8192

   #endif /* GL_GENERATE_MIPMAP_SGIS */

#endif /* GL_SGIS_generate_mipmap */


/* ---[ GL_SGIS_multisample ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/multisample.txt */

#ifndef GL_SGIS_multisample

   #define GL_SGIS_multisample 1
   #define GL_SGIS_multisample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_MULTISAMPLE_SGIS

      #define GL_MULTISAMPLE_SGIS                                         0x809d
      #define GL_SAMPLE_ALPHA_TO_MASK_SGIS                                0x809e
      #define GL_SAMPLE_ALPHA_TO_ONE_SGIS                                 0x809f
      #define GL_SAMPLE_MASK_SGIS                                         0x80a0
      #define GL_1PASS_SGIS                                               0x80a1
      #define GL_2PASS_0_SGIS                                             0x80a2
      #define GL_2PASS_1_SGIS                                             0x80a3
      #define GL_4PASS_0_SGIS                                             0x80a4
      #define GL_4PASS_1_SGIS                                             0x80a5
      #define GL_4PASS_2_SGIS                                             0x80a6
      #define GL_4PASS_3_SGIS                                             0x80a7
      #define GL_SAMPLE_BUFFERS_SGIS                                      0x80a8
      #define GL_SAMPLES_SGIS                                             0x80a9
      #define GL_SAMPLE_MASK_VALUE_SGIS                                   0x80aa
      #define GL_SAMPLE_MASK_INVERT_SGIS                                  0x80ab
      #define GL_SAMPLE_PATTERN_SGIS                                      0x80ac

   #endif /* GL_MULTISAMPLE_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glSampleMaskSGIS                                 OGLEXT_MAKEGLNAME(SampleMaskSGIS)
      #define glSamplePatternSGIS                              OGLEXT_MAKEGLNAME(SamplePatternSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glSampleMaskSGIS(GLclampf, GLboolean);
      GLAPI GLvoid            glSamplePatternSGIS(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_multisample */


/* ---[ GL_SGIS_multitexture ]------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIS_multitexture

   #define GL_SGIS_multitexture 1
   #define GL_SGIS_multitexture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SELECTED_TEXTURE_SGIS

      #define GL_SELECTED_TEXTURE_SGIS                                    0x83c0
      #define GL_SELECTED_TEXTURE_COORD_SET_SGIS                          0x83c1
      #define GL_SELECTED_TEXTURE_TRANSFORM_SGIS                          0x83c2
      #define GL_MAX_TEXTURES_SGIS                                        0x83c3
      #define GL_MAX_TEXTURE_COORD_SETS_SGIS                              0x83c4
      #define GL_TEXTURE_ENV_COORD_SET_SGIS                               0x83c5
      #define GL_TEXTURE0_SGIS                                            0x83c6
      #define GL_TEXTURE1_SGIS                                            0x83c7
      #define GL_TEXTURE2_SGIS                                            0x83c8
      #define GL_TEXTURE3_SGIS                                            0x83c9

   #endif /* GL_SELECTED_TEXTURE_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glInterleavedTextureCoordSetsSGIS                OGLEXT_MAKEGLNAME(InterleavedTextureCoordSetsSGIS)
      #define glMultiTexCoord1dSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord1dSGIS)
      #define glMultiTexCoord1dvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord1dvSGIS)
      #define glMultiTexCoord1fSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord1fSGIS)
      #define glMultiTexCoord1fvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord1fvSGIS)
      #define glMultiTexCoord1iSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord1iSGIS)
      #define glMultiTexCoord1ivSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord1ivSGIS)
      #define glMultiTexCoord1sSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord1sSGIS)
      #define glMultiTexCoord1svSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord1svSGIS)
      #define glMultiTexCoord2dSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord2dSGIS)
      #define glMultiTexCoord2dvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord2dvSGIS)
      #define glMultiTexCoord2fSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord2fSGIS)
      #define glMultiTexCoord2fvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord2fvSGIS)
      #define glMultiTexCoord2iSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord2iSGIS)
      #define glMultiTexCoord2ivSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord2ivSGIS)
      #define glMultiTexCoord2sSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord2sSGIS)
      #define glMultiTexCoord2svSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord2svSGIS)
      #define glMultiTexCoord3dSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord3dSGIS)
      #define glMultiTexCoord3dvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord3dvSGIS)
      #define glMultiTexCoord3fSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord3fSGIS)
      #define glMultiTexCoord3fvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord3fvSGIS)
      #define glMultiTexCoord3iSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord3iSGIS)
      #define glMultiTexCoord3ivSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord3ivSGIS)
      #define glMultiTexCoord3sSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord3sSGIS)
      #define glMultiTexCoord3svSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord3svSGIS)
      #define glMultiTexCoord4dSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord4dSGIS)
      #define glMultiTexCoord4dvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord4dvSGIS)
      #define glMultiTexCoord4fSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord4fSGIS)
      #define glMultiTexCoord4fvSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord4fvSGIS)
      #define glMultiTexCoord4iSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord4iSGIS)
      #define glMultiTexCoord4ivSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord4ivSGIS)
      #define glMultiTexCoord4sSGIS                            OGLEXT_MAKEGLNAME(MultiTexCoord4sSGIS)
      #define glMultiTexCoord4svSGIS                           OGLEXT_MAKEGLNAME(MultiTexCoord4svSGIS)
      #define glSelectTextureCoordSetSGIS                      OGLEXT_MAKEGLNAME(SelectTextureCoordSetSGIS)
      #define glSelectTextureSGIS                              OGLEXT_MAKEGLNAME(SelectTextureSGIS)
      #define glSelectTextureTransformSGIS                     OGLEXT_MAKEGLNAME(SelectTextureTransformSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glInterleavedTextureCoordSetsSGIS(GLint);
      GLAPI GLvoid            glMultiTexCoord1dSGIS(GLenum, GLdouble);
      GLAPI GLvoid            glMultiTexCoord1dvSGIS(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord1fSGIS(GLenum, GLfloat);
      GLAPI GLvoid            glMultiTexCoord1fvSGIS(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord1iSGIS(GLenum, GLint);
      GLAPI GLvoid            glMultiTexCoord1ivSGIS(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord1sSGIS(GLenum, GLshort);
      GLAPI GLvoid            glMultiTexCoord1svSGIS(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord2dSGIS(GLenum, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord2dvSGIS(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord2fSGIS(GLenum, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord2fvSGIS(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord2iSGIS(GLenum, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord2ivSGIS(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord2sSGIS(GLenum, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord2svSGIS(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord3dSGIS(GLenum, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord3dvSGIS(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord3fSGIS(GLenum, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord3fvSGIS(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord3iSGIS(GLenum, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord3ivSGIS(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord3sSGIS(GLenum, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord3svSGIS(GLenum, GLshort const *);
      GLAPI GLvoid            glMultiTexCoord4dSGIS(GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
      GLAPI GLvoid            glMultiTexCoord4dvSGIS(GLenum, GLdouble const *);
      GLAPI GLvoid            glMultiTexCoord4fSGIS(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glMultiTexCoord4fvSGIS(GLenum, GLfloat const *);
      GLAPI GLvoid            glMultiTexCoord4iSGIS(GLenum, GLint, GLint, GLint, GLint);
      GLAPI GLvoid            glMultiTexCoord4ivSGIS(GLenum, GLint const *);
      GLAPI GLvoid            glMultiTexCoord4sSGIS(GLenum, GLshort, GLshort, GLshort, GLshort);
      GLAPI GLvoid            glMultiTexCoord4svSGIS(GLenum, GLshort const *);
      GLAPI GLvoid            glSelectTextureCoordSetSGIS(GLenum);
      GLAPI GLvoid            glSelectTextureSGIS(GLenum);
      GLAPI GLvoid            glSelectTextureTransformSGIS(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_multitexture */


/* ---[ GL_SGIS_pixel_texture ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/pixel_texture.txt */

#ifndef GL_SGIS_pixel_texture

   #define GL_SGIS_pixel_texture 1
   #define GL_SGIS_pixel_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PIXEL_TEXTURE_SGIS

      #define GL_PIXEL_TEXTURE_SGIS                                       0x8353
      #define GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS                           0x8354
      #define GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS                         0x8355
      #define GL_PIXEL_GROUP_COLOR_SGIS                                   0x8356

   #endif /* GL_PIXEL_TEXTURE_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetPixelTexGenParameterfvSGIS                  OGLEXT_MAKEGLNAME(GetPixelTexGenParameterfvSGIS)
      #define glGetPixelTexGenParameterivSGIS                  OGLEXT_MAKEGLNAME(GetPixelTexGenParameterivSGIS)
      #define glPixelTexGenParameterfSGIS                      OGLEXT_MAKEGLNAME(PixelTexGenParameterfSGIS)
      #define glPixelTexGenParameterfvSGIS                     OGLEXT_MAKEGLNAME(PixelTexGenParameterfvSGIS)
      #define glPixelTexGenParameteriSGIS                      OGLEXT_MAKEGLNAME(PixelTexGenParameteriSGIS)
      #define glPixelTexGenParameterivSGIS                     OGLEXT_MAKEGLNAME(PixelTexGenParameterivSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetPixelTexGenParameterfvSGIS(GLenum, GLfloat *);
      GLAPI GLvoid            glGetPixelTexGenParameterivSGIS(GLenum, GLint *);
      GLAPI GLvoid            glPixelTexGenParameterfSGIS(GLenum, GLfloat);
      GLAPI GLvoid            glPixelTexGenParameterfvSGIS(GLenum, GLfloat const *);
      GLAPI GLvoid            glPixelTexGenParameteriSGIS(GLenum, GLint);
      GLAPI GLvoid            glPixelTexGenParameterivSGIS(GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_pixel_texture */


/* ---[ GL_SGIS_point_line_texgen ]-------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIS_point_line_texgen

   #define GL_SGIS_point_line_texgen 1
   #define GL_SGIS_point_line_texgen_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_EYE_DISTANCE_TO_POINT_SGIS

      #define GL_EYE_DISTANCE_TO_POINT_SGIS                               0x81f0
      #define GL_OBJECT_DISTANCE_TO_POINT_SGIS                            0x81f1
      #define GL_EYE_DISTANCE_TO_LINE_SGIS                                0x81f2
      #define GL_OBJECT_DISTANCE_TO_LINE_SGIS                             0x81f3
      #define GL_EYE_POINT_SGIS                                           0x81f4
      #define GL_OBJECT_POINT_SGIS                                        0x81f5
      #define GL_EYE_LINE_SGIS                                            0x81f6
      #define GL_OBJECT_LINE_SGIS                                         0x81f7

   #endif /* GL_EYE_DISTANCE_TO_POINT_SGIS */

#endif /* GL_SGIS_point_line_texgen */


/* ---[ GL_SGIS_point_parameters ]--------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIS_point_parameters

   #define GL_SGIS_point_parameters 1
   #define GL_SGIS_point_parameters_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_POINT_SIZE_MIN_SGIS

      #define GL_POINT_SIZE_MIN_SGIS                                      0x8126
      #define GL_POINT_SIZE_MAX_SGIS                                      0x8127
      #define GL_POINT_FADE_THRESHOLD_SIZE_SGIS                           0x8128
      #define GL_DISTANCE_ATTENUATION_SGIS                                0x8129

   #endif /* GL_POINT_SIZE_MIN_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPointParameterfSGIS                            OGLEXT_MAKEGLNAME(PointParameterfSGIS)
      #define glPointParameterfvSGIS                           OGLEXT_MAKEGLNAME(PointParameterfvSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPointParameterfSGIS(GLenum, GLfloat);
      GLAPI GLvoid            glPointParameterfvSGIS(GLenum, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_point_parameters */


/* ---[ GL_SGIS_sharpen_texture ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/sharpen_texture.txt */

#ifndef GL_SGIS_sharpen_texture

   #define GL_SGIS_sharpen_texture 1
   #define GL_SGIS_sharpen_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_LINEAR_SHARPEN_SGIS

      #define GL_LINEAR_SHARPEN_SGIS                                      0x80ad
      #define GL_LINEAR_SHARPEN_ALPHA_SGIS                                0x80ae
      #define GL_LINEAR_SHARPEN_COLOR_SGIS                                0x80af
      #define GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS                         0x80b0

   #endif /* GL_LINEAR_SHARPEN_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetSharpenTexFuncSGIS                          OGLEXT_MAKEGLNAME(GetSharpenTexFuncSGIS)
      #define glSharpenTexFuncSGIS                             OGLEXT_MAKEGLNAME(SharpenTexFuncSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetSharpenTexFuncSGIS(GLenum, GLfloat *);
      GLAPI GLvoid            glSharpenTexFuncSGIS(GLenum, GLsizei, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_sharpen_texture */


/* ---[ GL_SGIS_texture4D ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/texture4D.txt */

#ifndef GL_SGIS_texture4D

   #define GL_SGIS_texture4D 1
   #define GL_SGIS_texture4D_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PACK_SKIP_VOLUMES_SGIS

      #define GL_PACK_SKIP_VOLUMES_SGIS                                   0x8130
      #define GL_PACK_IMAGE_DEPTH_SGIS                                    0x8131
      #define GL_UNPACK_SKIP_VOLUMES_SGIS                                 0x8132
      #define GL_UNPACK_IMAGE_DEPTH_SGIS                                  0x8133
      #define GL_TEXTURE_4D_SGIS                                          0x8134
      #define GL_PROXY_TEXTURE_4D_SGIS                                    0x8135
      #define GL_TEXTURE_4DSIZE_SGIS                                      0x8136
      #define GL_TEXTURE_WRAP_Q_SGIS                                      0x8137
      #define GL_MAX_4D_TEXTURE_SIZE_SGIS                                 0x8138
      #define GL_TEXTURE_4D_BINDING_SGIS                                  0x814f

   #endif /* GL_PACK_SKIP_VOLUMES_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glTexImage4DSGIS                                 OGLEXT_MAKEGLNAME(TexImage4DSGIS)
      #define glTexSubImage4DSGIS                              OGLEXT_MAKEGLNAME(TexSubImage4DSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glTexImage4DSGIS(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glTexSubImage4DSGIS(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_texture4D */


/* ---[ GL_SGIS_texture_border_clamp ]----------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/texture_border_clamp.txt */

#ifndef GL_SGIS_texture_border_clamp

   #define GL_SGIS_texture_border_clamp 1
   #define GL_SGIS_texture_border_clamp_OGLEXT 1

#endif /* GL_SGIS_texture_border_clamp */


/* ---[ GL_SGIS_texture_color_mask ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/texture_color_mask.txt */

#ifndef GL_SGIS_texture_color_mask

   #define GL_SGIS_texture_color_mask 1
   #define GL_SGIS_texture_color_mask_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_COLOR_WRITEMASK_SGIS

      #define GL_TEXTURE_COLOR_WRITEMASK_SGIS                             0x81ef

   #endif /* GL_TEXTURE_COLOR_WRITEMASK_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glTextureColorMaskSGIS                           OGLEXT_MAKEGLNAME(TextureColorMaskSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glTextureColorMaskSGIS(GLboolean, GLboolean, GLboolean, GLboolean);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_texture_color_mask */


/* ---[ GL_SGIS_texture_edge_clamp ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/texture_edge_clamp.txt */

#ifndef GL_SGIS_texture_edge_clamp

   #define GL_SGIS_texture_edge_clamp 1
   #define GL_SGIS_texture_edge_clamp_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CLAMP_TO_EDGE_SGIS

      #define GL_CLAMP_TO_EDGE_SGIS                                       0x812f

   #endif /* GL_CLAMP_TO_EDGE_SGIS */

#endif /* GL_SGIS_texture_edge_clamp */


/* ---[ GL_SGIS_texture_filter4 ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/texture_filter4.txt */

#ifndef GL_SGIS_texture_filter4

   #define GL_SGIS_texture_filter4 1
   #define GL_SGIS_texture_filter4_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FILTER4_SGIS

      #define GL_FILTER4_SGIS                                             0x8146
      #define GL_TEXTURE_FILTER4_SIZE_SGIS                                0x8147

   #endif /* GL_FILTER4_SGIS */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetTexFilterFuncSGIS                           OGLEXT_MAKEGLNAME(GetTexFilterFuncSGIS)
      #define glTexFilterFuncSGIS                              OGLEXT_MAKEGLNAME(TexFilterFuncSGIS)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetTexFilterFuncSGIS(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glTexFilterFuncSGIS(GLenum, GLenum, GLsizei, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIS_texture_filter4 */


/* ---[ GL_SGIS_texture_lod ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIS/texture_lod.txt */

#ifndef GL_SGIS_texture_lod

   #define GL_SGIS_texture_lod 1
   #define GL_SGIS_texture_lod_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_MIN_LOD_SGIS

      #define GL_TEXTURE_MIN_LOD_SGIS                                     0x813a
      #define GL_TEXTURE_MAX_LOD_SGIS                                     0x813b
      #define GL_TEXTURE_BASE_LEVEL_SGIS                                  0x813c
      #define GL_TEXTURE_MAX_LEVEL_SGIS                                   0x813d

   #endif /* GL_TEXTURE_MIN_LOD_SGIS */

#endif /* GL_SGIS_texture_lod */


/* ---[ GL_SGIX_async ]-------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/async.txt */

#ifndef GL_SGIX_async

   #define GL_SGIX_async 1
   #define GL_SGIX_async_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ASYNC_MARKER_SGIX

      #define GL_ASYNC_MARKER_SGIX                                        0x8329

   #endif /* GL_ASYNC_MARKER_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glAsyncMarkerSGIX                                OGLEXT_MAKEGLNAME(AsyncMarkerSGIX)
      #define glDeleteAsyncMarkersSGIX                         OGLEXT_MAKEGLNAME(DeleteAsyncMarkersSGIX)
      #define glFinishAsyncSGIX                                OGLEXT_MAKEGLNAME(FinishAsyncSGIX)
      #define glGenAsyncMarkersSGIX                            OGLEXT_MAKEGLNAME(GenAsyncMarkersSGIX)
      #define glIsAsyncMarkerSGIX                              OGLEXT_MAKEGLNAME(IsAsyncMarkerSGIX)
      #define glPollAsyncSGIX                                  OGLEXT_MAKEGLNAME(PollAsyncSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glAsyncMarkerSGIX(GLuint);
      GLAPI GLvoid            glDeleteAsyncMarkersSGIX(GLuint, GLsizei);
      GLAPI GLint             glFinishAsyncSGIX(GLuint *);
      GLAPI GLuint            glGenAsyncMarkersSGIX(GLsizei);
      GLAPI GLboolean         glIsAsyncMarkerSGIX(GLuint);
      GLAPI GLint             glPollAsyncSGIX(GLuint *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_async */


/* ---[ GL_SGIX_async_histogram ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/async_histogram.txt */

#ifndef GL_SGIX_async_histogram

   #define GL_SGIX_async_histogram 1
   #define GL_SGIX_async_histogram_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ASYNC_HISTOGRAM_SGIX

      #define GL_ASYNC_HISTOGRAM_SGIX                                     0x832c
      #define GL_MAX_ASYNC_HISTOGRAM_SGIX                                 0x832d

   #endif /* GL_ASYNC_HISTOGRAM_SGIX */

#endif /* GL_SGIX_async_histogram */


/* ---[ GL_SGIX_async_pixel ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/async_pixel.txt */

#ifndef GL_SGIX_async_pixel

   #define GL_SGIX_async_pixel 1
   #define GL_SGIX_async_pixel_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ASYNC_TEX_IMAGE_SGIX

      #define GL_ASYNC_TEX_IMAGE_SGIX                                     0x835c
      #define GL_ASYNC_DRAW_PIXELS_SGIX                                   0x835d
      #define GL_ASYNC_READ_PIXELS_SGIX                                   0x835e
      #define GL_MAX_ASYNC_TEX_IMAGE_SGIX                                 0x835f
      #define GL_MAX_ASYNC_DRAW_PIXELS_SGIX                               0x8360
      #define GL_MAX_ASYNC_READ_PIXELS_SGIX                               0x8361

   #endif /* GL_ASYNC_TEX_IMAGE_SGIX */

#endif /* GL_SGIX_async_pixel */


/* ---[ GL_SGIX_blend_alpha_minmax ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/blend_alpha_minmax.txt */

#ifndef GL_SGIX_blend_alpha_minmax

   #define GL_SGIX_blend_alpha_minmax 1
   #define GL_SGIX_blend_alpha_minmax_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_ALPHA_MIN_SGIX

      #define GL_ALPHA_MIN_SGIX                                           0x8320
      #define GL_ALPHA_MAX_SGIX                                           0x8321

   #endif /* GL_ALPHA_MIN_SGIX */

#endif /* GL_SGIX_blend_alpha_minmax */


/* ---[ GL_SGIX_calligraphic_fragment ]---------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_calligraphic_fragment

   #define GL_SGIX_calligraphic_fragment 1
   #define GL_SGIX_calligraphic_fragment_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CALLIGRAPHIC_FRAGMENT_SGIX

      #define GL_CALLIGRAPHIC_FRAGMENT_SGIX                               0x8183

   #endif /* GL_CALLIGRAPHIC_FRAGMENT_SGIX */

#endif /* GL_SGIX_calligraphic_fragment */


/* ---[ GL_SGIX_clipmap ]------------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/clipmap.txt */

#ifndef GL_SGIX_clipmap

   #define GL_SGIX_clipmap 1
   #define GL_SGIX_clipmap_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_LINEAR_CLIPMAP_LINEAR_SGIX

      #define GL_LINEAR_CLIPMAP_LINEAR_SGIX                               0x8170
      #define GL_TEXTURE_CLIPMAP_CENTER_SGIX                              0x8171
      #define GL_TEXTURE_CLIPMAP_FRAME_SGIX                               0x8172
      #define GL_TEXTURE_CLIPMAP_OFFSET_SGIX                              0x8173
      #define GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX                       0x8174
      #define GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX                          0x8175
      #define GL_TEXTURE_CLIPMAP_DEPTH_SGIX                               0x8176
      #define GL_MAX_CLIPMAP_DEPTH_SGIX                                   0x8177
      #define GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX                           0x8178
      #define GL_NEAREST_CLIPMAP_NEAREST_SGIX                             0x844d
      #define GL_NEAREST_CLIPMAP_LINEAR_SGIX                              0x844e
      #define GL_LINEAR_CLIPMAP_NEAREST_SGIX                              0x844f

   #endif /* GL_LINEAR_CLIPMAP_LINEAR_SGIX */

#endif /* GL_SGIX_clipmap */


/* ---[ GL_SGIX_convolution_accuracy ]----------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_convolution_accuracy

   #define GL_SGIX_convolution_accuracy 1
   #define GL_SGIX_convolution_accuracy_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_CONVOLUTION_HINT_SGIX

      #define GL_CONVOLUTION_HINT_SGIX                                    0x8316

   #endif /* GL_CONVOLUTION_HINT_SGIX */

#endif /* GL_SGIX_convolution_accuracy */


/* ---[ GL_SGIX_depth_pass_instrument ]---------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_depth_pass_instrument

   #define GL_SGIX_depth_pass_instrument 1
   #define GL_SGIX_depth_pass_instrument_OGLEXT 1

#endif /* GL_SGIX_depth_pass_instrument */


/* ---[ GL_SGIX_depth_texture ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/depth_texture.txt */

#ifndef GL_SGIX_depth_texture

   #define GL_SGIX_depth_texture 1
   #define GL_SGIX_depth_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_DEPTH_COMPONENT16_SGIX

      #define GL_DEPTH_COMPONENT16_SGIX                                   0x81a5
      #define GL_DEPTH_COMPONENT24_SGIX                                   0x81a6
      #define GL_DEPTH_COMPONENT32_SGIX                                   0x81a7

   #endif /* GL_DEPTH_COMPONENT16_SGIX */

#endif /* GL_SGIX_depth_texture */


/* ---[ GL_SGIX_flush_raster ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/flush_raster.txt */

#ifndef GL_SGIX_flush_raster

   #define GL_SGIX_flush_raster 1
   #define GL_SGIX_flush_raster_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFlushRasterSGIX                                OGLEXT_MAKEGLNAME(FlushRasterSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFlushRasterSGIX();

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_flush_raster */


/* ---[ GL_SGIX_fog_offset ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/fog_offset.txt */

#ifndef GL_SGIX_fog_offset

   #define GL_SGIX_fog_offset 1
   #define GL_SGIX_fog_offset_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FOG_OFFSET_SGIX

      #define GL_FOG_OFFSET_SGIX                                          0x8198
      #define GL_FOG_OFFSET_VALUE_SGIX                                    0x8199

   #endif /* GL_FOG_OFFSET_SGIX */

#endif /* GL_SGIX_fog_offset */


/* ---[ GL_SGIX_fog_scale ]---------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_fog_scale

   #define GL_SGIX_fog_scale 1
   #define GL_SGIX_fog_scale_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FOG_SCALE_SGIX

      #define GL_FOG_SCALE_SGIX                                           0x81fc
      #define GL_FOG_SCALE_VALUE_SGIX                                     0x81fd

   #endif /* GL_FOG_SCALE_SGIX */

#endif /* GL_SGIX_fog_scale */


/* ---[ GL_SGIX_fragment_lighting ]-------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_fragment_lighting

   #define GL_SGIX_fragment_lighting 1
   #define GL_SGIX_fragment_lighting_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FRAGMENT_LIGHTING_SGIX

      #define GL_FRAGMENT_LIGHTING_SGIX                                   0x8400
      #define GL_FRAGMENT_COLOR_MATERIAL_SGIX                             0x8401
      #define GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX                        0x8402
      #define GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX                   0x8403
      #define GL_MAX_FRAGMENT_LIGHTS_SGIX                                 0x8404
      #define GL_MAX_ACTIVE_LIGHTS_SGIX                                   0x8405
      #define GL_CURRENT_RASTER_NORMAL_SGIX                               0x8406
      #define GL_LIGHT_ENV_MODE_SGIX                                      0x8407
      #define GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX                   0x8408
      #define GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX                       0x8409
      #define GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX                        0x840a
      #define GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX           0x840b
      #define GL_FRAGMENT_LIGHT0_SGIX                                     0x840c
      #define GL_FRAGMENT_LIGHT1_SGIX                                     0x840d
      #define GL_FRAGMENT_LIGHT2_SGIX                                     0x840e
      #define GL_FRAGMENT_LIGHT3_SGIX                                     0x840f
      #define GL_FRAGMENT_LIGHT4_SGIX                                     0x8410
      #define GL_FRAGMENT_LIGHT5_SGIX                                     0x8411
      #define GL_FRAGMENT_LIGHT6_SGIX                                     0x8412
      #define GL_FRAGMENT_LIGHT7_SGIX                                     0x8413

   #endif /* GL_FRAGMENT_LIGHTING_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFragmentColorMaterialSGIX                      OGLEXT_MAKEGLNAME(FragmentColorMaterialSGIX)
      #define glFragmentLightfSGIX                             OGLEXT_MAKEGLNAME(FragmentLightfSGIX)
      #define glFragmentLightfvSGIX                            OGLEXT_MAKEGLNAME(FragmentLightfvSGIX)
      #define glFragmentLightiSGIX                             OGLEXT_MAKEGLNAME(FragmentLightiSGIX)
      #define glFragmentLightivSGIX                            OGLEXT_MAKEGLNAME(FragmentLightivSGIX)
      #define glFragmentLightModelfSGIX                        OGLEXT_MAKEGLNAME(FragmentLightModelfSGIX)
      #define glFragmentLightModelfvSGIX                       OGLEXT_MAKEGLNAME(FragmentLightModelfvSGIX)
      #define glFragmentLightModeliSGIX                        OGLEXT_MAKEGLNAME(FragmentLightModeliSGIX)
      #define glFragmentLightModelivSGIX                       OGLEXT_MAKEGLNAME(FragmentLightModelivSGIX)
      #define glFragmentMaterialfSGIX                          OGLEXT_MAKEGLNAME(FragmentMaterialfSGIX)
      #define glFragmentMaterialfvSGIX                         OGLEXT_MAKEGLNAME(FragmentMaterialfvSGIX)
      #define glFragmentMaterialiSGIX                          OGLEXT_MAKEGLNAME(FragmentMaterialiSGIX)
      #define glFragmentMaterialivSGIX                         OGLEXT_MAKEGLNAME(FragmentMaterialivSGIX)
      #define glGetFragmentLightfvSGIX                         OGLEXT_MAKEGLNAME(GetFragmentLightfvSGIX)
      #define glGetFragmentLightivSGIX                         OGLEXT_MAKEGLNAME(GetFragmentLightivSGIX)
      #define glGetFragmentMaterialfvSGIX                      OGLEXT_MAKEGLNAME(GetFragmentMaterialfvSGIX)
      #define glGetFragmentMaterialivSGIX                      OGLEXT_MAKEGLNAME(GetFragmentMaterialivSGIX)
      #define glLightEnviSGIX                                  OGLEXT_MAKEGLNAME(LightEnviSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFragmentColorMaterialSGIX(GLenum, GLenum);
      GLAPI GLvoid            glFragmentLightfSGIX(GLenum, GLenum, GLfloat);
      GLAPI GLvoid            glFragmentLightfvSGIX(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glFragmentLightiSGIX(GLenum, GLenum, GLint);
      GLAPI GLvoid            glFragmentLightivSGIX(GLenum, GLenum, GLint const *);
      GLAPI GLvoid            glFragmentLightModelfSGIX(GLenum, GLfloat);
      GLAPI GLvoid            glFragmentLightModelfvSGIX(GLenum, GLfloat const *);
      GLAPI GLvoid            glFragmentLightModeliSGIX(GLenum, GLint);
      GLAPI GLvoid            glFragmentLightModelivSGIX(GLenum, GLint const *);
      GLAPI GLvoid            glFragmentMaterialfSGIX(GLenum, GLenum, GLfloat);
      GLAPI GLvoid            glFragmentMaterialfvSGIX(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glFragmentMaterialiSGIX(GLenum, GLenum, GLint);
      GLAPI GLvoid            glFragmentMaterialivSGIX(GLenum, GLenum, GLint const *);
      GLAPI GLvoid            glGetFragmentLightfvSGIX(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetFragmentLightivSGIX(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetFragmentMaterialfvSGIX(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetFragmentMaterialivSGIX(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glLightEnviSGIX(GLenum, GLint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_fragment_lighting */


/* ---[ GL_SGIX_framezoom ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/framezoom.txt */

#ifndef GL_SGIX_framezoom

   #define GL_SGIX_framezoom 1
   #define GL_SGIX_framezoom_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_FRAMEZOOM_SGIX

      #define GL_FRAMEZOOM_SGIX                                           0x818b
      #define GL_FRAMEZOOM_FACTOR_SGIX                                    0x818c
      #define GL_MAX_FRAMEZOOM_FACTOR_SGIX                                0x818d

   #endif /* GL_FRAMEZOOM_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFrameZoomSGIX                                  OGLEXT_MAKEGLNAME(FrameZoomSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFrameZoomSGIX(GLint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_framezoom */


/* ---[ GL_SGIX_igloo_interface ]---------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_igloo_interface

   #define GL_SGIX_igloo_interface 1
   #define GL_SGIX_igloo_interface_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glIglooInterfaceSGIX                             OGLEXT_MAKEGLNAME(IglooInterfaceSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glIglooInterfaceSGIX(GLenum, GLvoid const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_igloo_interface */


/* ---[ GL_SGIX_instruments ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/instruments.txt */

#ifndef GL_SGIX_instruments

   #define GL_SGIX_instruments 1
   #define GL_SGIX_instruments_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_INSTRUMENT_BUFFER_POINTER_SGIX

      #define GL_INSTRUMENT_BUFFER_POINTER_SGIX                           0x8180
      #define GL_INSTRUMENT_MEASUREMENTS_SGIX                             0x8181

   #endif /* GL_INSTRUMENT_BUFFER_POINTER_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetInstrumentsSGIX                             OGLEXT_MAKEGLNAME(GetInstrumentsSGIX)
      #define glInstrumentsBufferSGIX                          OGLEXT_MAKEGLNAME(InstrumentsBufferSGIX)
      #define glPollInstrumentsSGIX                            OGLEXT_MAKEGLNAME(PollInstrumentsSGIX)
      #define glReadInstrumentsSGIX                            OGLEXT_MAKEGLNAME(ReadInstrumentsSGIX)
      #define glStartInstrumentsSGIX                           OGLEXT_MAKEGLNAME(StartInstrumentsSGIX)
      #define glStopInstrumentsSGIX                            OGLEXT_MAKEGLNAME(StopInstrumentsSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLint             glGetInstrumentsSGIX();
      GLAPI GLvoid            glInstrumentsBufferSGIX(GLsizei, GLint *);
      GLAPI GLint             glPollInstrumentsSGIX(GLint *);
      GLAPI GLvoid            glReadInstrumentsSGIX(GLint);
      GLAPI GLvoid            glStartInstrumentsSGIX();
      GLAPI GLvoid            glStopInstrumentsSGIX(GLint);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_instruments */


/* ---[ GL_SGIX_interlace ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/interlace.txt */

#ifndef GL_SGIX_interlace

   #define GL_SGIX_interlace 1
   #define GL_SGIX_interlace_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_INTERLACE_SGIX

      #define GL_INTERLACE_SGIX                                           0x8094

   #endif /* GL_INTERLACE_SGIX */

#endif /* GL_SGIX_interlace */


/* ---[ GL_SGIX_ir_instrument1 ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/ir_instrument1.txt */

#ifndef GL_SGIX_ir_instrument1

   #define GL_SGIX_ir_instrument1 1
   #define GL_SGIX_ir_instrument1_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_IR_INSTRUMENT1_SGIX

      #define GL_IR_INSTRUMENT1_SGIX                                      0x817f

   #endif /* GL_IR_INSTRUMENT1_SGIX */

#endif /* GL_SGIX_ir_instrument1 */


/* ---[ GL_SGIX_list_priority ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/list_priority.txt */

#ifndef GL_SGIX_list_priority

   #define GL_SGIX_list_priority 1
   #define GL_SGIX_list_priority_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_LIST_PRIORITY_SGIX

      #define GL_LIST_PRIORITY_SGIX                                       0x8182

   #endif /* GL_LIST_PRIORITY_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGetListParameterfvSGIX                         OGLEXT_MAKEGLNAME(GetListParameterfvSGIX)
      #define glGetListParameterivSGIX                         OGLEXT_MAKEGLNAME(GetListParameterivSGIX)
      #define glListParameterfSGIX                             OGLEXT_MAKEGLNAME(ListParameterfSGIX)
      #define glListParameterfvSGIX                            OGLEXT_MAKEGLNAME(ListParameterfvSGIX)
      #define glListParameteriSGIX                             OGLEXT_MAKEGLNAME(ListParameteriSGIX)
      #define glListParameterivSGIX                            OGLEXT_MAKEGLNAME(ListParameterivSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGetListParameterfvSGIX(GLuint, GLenum, GLfloat *);
      GLAPI GLvoid            glGetListParameterivSGIX(GLuint, GLenum, GLint *);
      GLAPI GLvoid            glListParameterfSGIX(GLuint, GLenum, GLfloat);
      GLAPI GLvoid            glListParameterfvSGIX(GLuint, GLenum, GLfloat const *);
      GLAPI GLvoid            glListParameteriSGIX(GLuint, GLenum, GLint);
      GLAPI GLvoid            glListParameterivSGIX(GLuint, GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_list_priority */


/* ---[ GL_SGIX_pixel_texture ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/sgix_pixel_texture.txt */

#ifndef GL_SGIX_pixel_texture

   #define GL_SGIX_pixel_texture 1
   #define GL_SGIX_pixel_texture_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PIXEL_TEX_GEN_SGIX

      #define GL_PIXEL_TEX_GEN_SGIX                                       0x8139
      #define GL_PIXEL_TEX_GEN_MODE_SGIX                                  0x832b

   #endif /* GL_PIXEL_TEX_GEN_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glPixelTexGenSGIX                                OGLEXT_MAKEGLNAME(PixelTexGenSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glPixelTexGenSGIX(GLenum);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_pixel_texture */


/* ---[ GL_SGIX_pixel_tiles ]-------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_pixel_tiles

   #define GL_SGIX_pixel_tiles 1
   #define GL_SGIX_pixel_tiles_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX

      #define GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX                           0x813e
      #define GL_PIXEL_TILE_CACHE_INCREMENT_SGIX                          0x813f
      #define GL_PIXEL_TILE_WIDTH_SGIX                                    0x8140
      #define GL_PIXEL_TILE_HEIGHT_SGIX                                   0x8141
      #define GL_PIXEL_TILE_GRID_WIDTH_SGIX                               0x8142
      #define GL_PIXEL_TILE_GRID_HEIGHT_SGIX                              0x8143
      #define GL_PIXEL_TILE_GRID_DEPTH_SGIX                               0x8144
      #define GL_PIXEL_TILE_CACHE_SIZE_SGIX                               0x8145

   #endif /* GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX */

#endif /* GL_SGIX_pixel_tiles */


/* ---[ GL_SGIX_polynomial_ffd ]----------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_polynomial_ffd

   #define GL_SGIX_polynomial_ffd 1
   #define GL_SGIX_polynomial_ffd_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_GEOMETRY_DEFORMATION_SGIX

      #define GL_GEOMETRY_DEFORMATION_SGIX                                0x8194
      #define GL_TEXTURE_DEFORMATION_SGIX                                 0x8195
      #define GL_DEFORMATIONS_MASK_SGIX                                   0x8196
      #define GL_MAX_DEFORMATION_ORDER_SGIX                               0x8197

   #endif /* GL_GEOMETRY_DEFORMATION_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDeformationMap3dSGIX                           OGLEXT_MAKEGLNAME(DeformationMap3dSGIX)
      #define glDeformationMap3fSGIX                           OGLEXT_MAKEGLNAME(DeformationMap3fSGIX)
      #define glDeformSGIX                                     OGLEXT_MAKEGLNAME(DeformSGIX)
      #define glLoadIdentityDeformationMapSGIX                 OGLEXT_MAKEGLNAME(LoadIdentityDeformationMapSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDeformationMap3dSGIX(GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, GLdouble const *);
      GLAPI GLvoid            glDeformationMap3fSGIX(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat const *);
      GLAPI GLvoid            glDeformSGIX(GLbitfield);
      GLAPI GLvoid            glLoadIdentityDeformationMapSGIX(GLbitfield);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_polynomial_ffd */


/* ---[ GL_SGIX_reference_plane ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/reference_plane.txt */

#ifndef GL_SGIX_reference_plane

   #define GL_SGIX_reference_plane 1
   #define GL_SGIX_reference_plane_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_REFERENCE_PLANE_SGIX

      #define GL_REFERENCE_PLANE_SGIX                                     0x817d
      #define GL_REFERENCE_PLANE_EQUATION_SGIX                            0x817e

   #endif /* GL_REFERENCE_PLANE_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glReferencePlaneSGIX                             OGLEXT_MAKEGLNAME(ReferencePlaneSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glReferencePlaneSGIX(GLdouble const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_reference_plane */


/* ---[ GL_SGIX_resample ]----------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/resample.txt */

#ifndef GL_SGIX_resample

   #define GL_SGIX_resample 1
   #define GL_SGIX_resample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PACK_RESAMPLE_SGIX

      #define GL_PACK_RESAMPLE_SGIX                                       0x842c
      #define GL_UNPACK_RESAMPLE_SGIX                                     0x842d
      #define GL_RESAMPLE_REPLICATE_SGIX                                  0x842e
      #define GL_RESAMPLE_ZERO_FILL_SGIX                                  0x842f
      #define GL_RESAMPLE_DECIMATE_SGIX                                   0x8430

   #endif /* GL_PACK_RESAMPLE_SGIX */

#endif /* GL_SGIX_resample */


/* ---[ GL_SGIX_scalebias_hint ]----------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_scalebias_hint

   #define GL_SGIX_scalebias_hint 1
   #define GL_SGIX_scalebias_hint_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SCALEBIAS_HINT_SGIX

      #define GL_SCALEBIAS_HINT_SGIX                                      0x8322

   #endif /* GL_SCALEBIAS_HINT_SGIX */

#endif /* GL_SGIX_scalebias_hint */


/* ---[ GL_SGIX_shadow ]------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/shadow.txt */

#ifndef GL_SGIX_shadow

   #define GL_SGIX_shadow 1
   #define GL_SGIX_shadow_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_COMPARE_SGIX

      #define GL_TEXTURE_COMPARE_SGIX                                     0x819a
      #define GL_TEXTURE_COMPARE_OPERATOR_SGIX                            0x819b
      #define GL_TEXTURE_LEQUAL_R_SGIX                                    0x819c
      #define GL_TEXTURE_GEQUAL_R_SGIX                                    0x819d

   #endif /* GL_TEXTURE_COMPARE_SGIX */

#endif /* GL_SGIX_shadow */


/* ---[ GL_SGIX_shadow_ambient ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/shadow_ambient.txt */

#ifndef GL_SGIX_shadow_ambient

   #define GL_SGIX_shadow_ambient 1
   #define GL_SGIX_shadow_ambient_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SHADOW_AMBIENT_SGIX

      #define GL_SHADOW_AMBIENT_SGIX                                      0x80bf

   #endif /* GL_SHADOW_AMBIENT_SGIX */

#endif /* GL_SGIX_shadow_ambient */


/* ---[ GL_SGIX_sprite ]------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/sprite.txt */

#ifndef GL_SGIX_sprite

   #define GL_SGIX_sprite 1
   #define GL_SGIX_sprite_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SPRITE_SGIX

      #define GL_SPRITE_SGIX                                              0x8148
      #define GL_SPRITE_MODE_SGIX                                         0x8149
      #define GL_SPRITE_AXIS_SGIX                                         0x814a
      #define GL_SPRITE_TRANSLATION_SGIX                                  0x814b
      #define GL_SPRITE_AXIAL_SGIX                                        0x814c
      #define GL_SPRITE_OBJECT_ALIGNED_SGIX                               0x814d
      #define GL_SPRITE_EYE_ALIGNED_SGIX                                  0x814e

   #endif /* GL_SPRITE_SGIX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glSpriteParameterfSGIX                           OGLEXT_MAKEGLNAME(SpriteParameterfSGIX)
      #define glSpriteParameterfvSGIX                          OGLEXT_MAKEGLNAME(SpriteParameterfvSGIX)
      #define glSpriteParameteriSGIX                           OGLEXT_MAKEGLNAME(SpriteParameteriSGIX)
      #define glSpriteParameterivSGIX                          OGLEXT_MAKEGLNAME(SpriteParameterivSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glSpriteParameterfSGIX(GLenum, GLfloat);
      GLAPI GLvoid            glSpriteParameterfvSGIX(GLenum, GLfloat const *);
      GLAPI GLvoid            glSpriteParameteriSGIX(GLenum, GLint);
      GLAPI GLvoid            glSpriteParameterivSGIX(GLenum, GLint const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_sprite */


/* ---[ GL_SGIX_subsample ]---------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_subsample

   #define GL_SGIX_subsample 1
   #define GL_SGIX_subsample_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_PACK_SUBSAMPLE_RATE_SGIX

      #define GL_PACK_SUBSAMPLE_RATE_SGIX                                 0x85a0
      #define GL_UNPACK_SUBSAMPLE_RATE_SGIX                               0x85a1
      #define GL_PIXEL_SUBSAMPLE_4444_SGIX                                0x85a2
      #define GL_PIXEL_SUBSAMPLE_2424_SGIX                                0x85a3
      #define GL_PIXEL_SUBSAMPLE_4242_SGIX                                0x85a4

   #endif /* GL_PACK_SUBSAMPLE_RATE_SGIX */

#endif /* GL_SGIX_subsample */


/* ---[ GL_SGIX_tag_sample_buffer ]-------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/tag_sample_buffer.txt */

#ifndef GL_SGIX_tag_sample_buffer

   #define GL_SGIX_tag_sample_buffer 1
   #define GL_SGIX_tag_sample_buffer_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glTagSampleBufferSGIX                            OGLEXT_MAKEGLNAME(TagSampleBufferSGIX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glTagSampleBufferSGIX();

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGIX_tag_sample_buffer */


/* ---[ GL_SGIX_texture_add_env ]---------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/texture_env_add.txt */

#ifndef GL_SGIX_texture_add_env

   #define GL_SGIX_texture_add_env 1
   #define GL_SGIX_texture_add_env_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_ENV_BIAS_SGIX

      #define GL_TEXTURE_ENV_BIAS_SGIX                                    0x80be

   #endif /* GL_TEXTURE_ENV_BIAS_SGIX */

#endif /* GL_SGIX_texture_add_env */


/* ---[ GL_SGIX_texture_coordinate_clamp ]------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/texture_coordinate_clamp.txt */

#ifndef GL_SGIX_texture_coordinate_clamp

   #define GL_SGIX_texture_coordinate_clamp 1
   #define GL_SGIX_texture_coordinate_clamp_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_MAX_CLAMP_S_SGIX

      #define GL_TEXTURE_MAX_CLAMP_S_SGIX                                 0x8369
      #define GL_TEXTURE_MAX_CLAMP_T_SGIX                                 0x836a
      #define GL_TEXTURE_MAX_CLAMP_R_SGIX                                 0x836b

   #endif /* GL_TEXTURE_MAX_CLAMP_S_SGIX */

#endif /* GL_SGIX_texture_coordinate_clamp */


/* ---[ GL_SGIX_texture_lod_bias ]--------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/texture_lod_bias.txt */

#ifndef GL_SGIX_texture_lod_bias

   #define GL_SGIX_texture_lod_bias 1
   #define GL_SGIX_texture_lod_bias_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_LOD_BIAS_S_SGIX

      #define GL_TEXTURE_LOD_BIAS_S_SGIX                                  0x818e
      #define GL_TEXTURE_LOD_BIAS_T_SGIX                                  0x818f
      #define GL_TEXTURE_LOD_BIAS_R_SGIX                                  0x8190

   #endif /* GL_TEXTURE_LOD_BIAS_S_SGIX */

#endif /* GL_SGIX_texture_lod_bias */


/* ---[ GL_SGIX_texture_multi_buffer ]----------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/texture_multi_buffer.txt */

#ifndef GL_SGIX_texture_multi_buffer

   #define GL_SGIX_texture_multi_buffer 1
   #define GL_SGIX_texture_multi_buffer_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_MULTI_BUFFER_HINT_SGIX

      #define GL_TEXTURE_MULTI_BUFFER_HINT_SGIX                           0x812e

   #endif /* GL_TEXTURE_MULTI_BUFFER_HINT_SGIX */

#endif /* GL_SGIX_texture_multi_buffer */


/* ---[ GL_SGIX_texture_scale_bias ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/texture_range.txt */

#ifndef GL_SGIX_texture_scale_bias

   #define GL_SGIX_texture_scale_bias 1
   #define GL_SGIX_texture_scale_bias_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_POST_TEXTURE_FILTER_BIAS_SGIX

      #define GL_POST_TEXTURE_FILTER_BIAS_SGIX                            0x8179
      #define GL_POST_TEXTURE_FILTER_SCALE_SGIX                           0x817a
      #define GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX                      0x817b
      #define GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX                     0x817c

   #endif /* GL_POST_TEXTURE_FILTER_BIAS_SGIX */

#endif /* GL_SGIX_texture_scale_bias */


/* ---[ GL_SGIX_texture_select ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/texture_scale_bias.txt */

#ifndef GL_SGIX_texture_select

   #define GL_SGIX_texture_select 1
   #define GL_SGIX_texture_select_OGLEXT 1

#endif /* GL_SGIX_texture_select */


/* ---[ GL_SGIX_vertex_preclip ]----------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/vertex_preclip.txt */

#ifndef GL_SGIX_vertex_preclip

   #define GL_SGIX_vertex_preclip 1
   #define GL_SGIX_vertex_preclip_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_VERTEX_PRECLIP_SGIX

      #define GL_VERTEX_PRECLIP_SGIX                                      0x83ee
      #define GL_VERTEX_PRECLIP_HINT_SGIX                                 0x83ef

   #endif /* GL_VERTEX_PRECLIP_SGIX */

#endif /* GL_SGIX_vertex_preclip */


/* ---[ GL_SGIX_ycrcb ]-------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGIX/ycrcb.txt */

#ifndef GL_SGIX_ycrcb

   #define GL_SGIX_ycrcb 1
   #define GL_SGIX_ycrcb_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_YCRCB_422_SGIX

      #define GL_YCRCB_422_SGIX                                           0x81bb
      #define GL_YCRCB_444_SGIX                                           0x81bc

   #endif /* GL_YCRCB_422_SGIX */

#endif /* GL_SGIX_ycrcb */


/* ---[ GL_SGIX_ycrcba ]------------------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_ycrcba

   #define GL_SGIX_ycrcba 1
   #define GL_SGIX_ycrcba_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_YCRCB_SGIX

      #define GL_YCRCB_SGIX                                               0x8318
      #define GL_YCRCBA_SGIX                                              0x8319

   #endif /* GL_YCRCB_SGIX */

#endif /* GL_SGIX_ycrcba */


/* ---[ GL_SGIX_ycrcb_subsample ]---------------------------------------------------------------------------- */

/*! link :  */

#ifndef GL_SGIX_ycrcb_subsample

   #define GL_SGIX_ycrcb_subsample 1
   #define GL_SGIX_ycrcb_subsample_OGLEXT 1

#endif /* GL_SGIX_ycrcb_subsample */


/* ---[ GL_SGI_color_matrix ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGI/color_matrix.txt */

#ifndef GL_SGI_color_matrix

   #define GL_SGI_color_matrix 1
   #define GL_SGI_color_matrix_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COLOR_MATRIX_SGI

      #define GL_COLOR_MATRIX_SGI                                         0x80b1
      #define GL_COLOR_MATRIX_STACK_DEPTH_SGI                             0x80b2
      #define GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI                         0x80b3
      #define GL_POST_COLOR_MATRIX_RED_SCALE_SGI                          0x80b4
      #define GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI                        0x80b5
      #define GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI                         0x80b6
      #define GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI                        0x80b7
      #define GL_POST_COLOR_MATRIX_RED_BIAS_SGI                           0x80b8
      #define GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI                         0x80b9
      #define GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI                          0x80ba
      #define GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI                         0x80bb

   #endif /* GL_COLOR_MATRIX_SGI */

#endif /* GL_SGI_color_matrix */


/* ---[ GL_SGI_color_table ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGI/color_table.txt */

#ifndef GL_SGI_color_table

   #define GL_SGI_color_table 1
   #define GL_SGI_color_table_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_COLOR_TABLE_SGI

      #define GL_COLOR_TABLE_SGI                                          0x80d0
      #define GL_POST_CONVOLUTION_COLOR_TABLE_SGI                         0x80d1
      #define GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI                        0x80d2
      #define GL_PROXY_COLOR_TABLE_SGI                                    0x80d3
      #define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI                   0x80d4
      #define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI                  0x80d5
      #define GL_COLOR_TABLE_SCALE_SGI                                    0x80d6
      #define GL_COLOR_TABLE_BIAS_SGI                                     0x80d7
      #define GL_COLOR_TABLE_FORMAT_SGI                                   0x80d8
      #define GL_COLOR_TABLE_WIDTH_SGI                                    0x80d9
      #define GL_COLOR_TABLE_RED_SIZE_SGI                                 0x80da
      #define GL_COLOR_TABLE_GREEN_SIZE_SGI                               0x80db
      #define GL_COLOR_TABLE_BLUE_SIZE_SGI                                0x80dc
      #define GL_COLOR_TABLE_ALPHA_SIZE_SGI                               0x80dd
      #define GL_COLOR_TABLE_LUMINANCE_SIZE_SGI                           0x80de
      #define GL_COLOR_TABLE_INTENSITY_SIZE_SGI                           0x80df

   #endif /* GL_COLOR_TABLE_SGI */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glColorTableParameterfvSGI                       OGLEXT_MAKEGLNAME(ColorTableParameterfvSGI)
      #define glColorTableParameterivSGI                       OGLEXT_MAKEGLNAME(ColorTableParameterivSGI)
      #define glColorTableSGI                                  OGLEXT_MAKEGLNAME(ColorTableSGI)
      #define glCopyColorTableSGI                              OGLEXT_MAKEGLNAME(CopyColorTableSGI)
      #define glGetColorTableParameterfvSGI                    OGLEXT_MAKEGLNAME(GetColorTableParameterfvSGI)
      #define glGetColorTableParameterivSGI                    OGLEXT_MAKEGLNAME(GetColorTableParameterivSGI)
      #define glGetColorTableSGI                               OGLEXT_MAKEGLNAME(GetColorTableSGI)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glColorTableParameterfvSGI(GLenum, GLenum, GLfloat const *);
      GLAPI GLvoid            glColorTableParameterivSGI(GLenum, GLenum, GLint const *);
      GLAPI GLvoid            glColorTableSGI(GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
      GLAPI GLvoid            glCopyColorTableSGI(GLenum, GLenum, GLint, GLint, GLsizei);
      GLAPI GLvoid            glGetColorTableParameterfvSGI(GLenum, GLenum, GLfloat *);
      GLAPI GLvoid            glGetColorTableParameterivSGI(GLenum, GLenum, GLint *);
      GLAPI GLvoid            glGetColorTableSGI(GLenum, GLenum, GLenum, GLvoid *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SGI_color_table */


/* ---[ GL_SGI_texture_color_table ]------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SGI/texture_color_table.txt */

#ifndef GL_SGI_texture_color_table

   #define GL_SGI_texture_color_table 1
   #define GL_SGI_texture_color_table_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_TEXTURE_COLOR_TABLE_SGI

      #define GL_TEXTURE_COLOR_TABLE_SGI                                  0x80bc
      #define GL_PROXY_TEXTURE_COLOR_TABLE_SGI                            0x80bd

   #endif /* GL_TEXTURE_COLOR_TABLE_SGI */

#endif /* GL_SGI_texture_color_table */


/* ---[ GL_SUNX_constant_data ]------------------------------------------------------------------------------ */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SUNX/constant_data.txt */

#ifndef GL_SUNX_constant_data

   #define GL_SUNX_constant_data 1
   #define GL_SUNX_constant_data_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_UNPACK_CONSTANT_DATA_SUNX

      #define GL_UNPACK_CONSTANT_DATA_SUNX                                0x81d5
      #define GL_TEXTURE_CONSTANT_DATA_SUNX                               0x81d6

   #endif /* GL_UNPACK_CONSTANT_DATA_SUNX */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glFinishTextureSUNX                              OGLEXT_MAKEGLNAME(FinishTextureSUNX)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glFinishTextureSUNX();

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SUNX_constant_data */


/* ---[ GL_SUN_convolution_border_modes ]-------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SUN/convolution_border_modes.txt */

#ifndef GL_SUN_convolution_border_modes

   #define GL_SUN_convolution_border_modes 1
   #define GL_SUN_convolution_border_modes_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_WRAP_BORDER_SUN

      #define GL_WRAP_BORDER_SUN                                          0x81d4

   #endif /* GL_WRAP_BORDER_SUN */

#endif /* GL_SUN_convolution_border_modes */


/* ---[ GL_SUN_global_alpha ]-------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SUN/global_alpha.txt */

#ifndef GL_SUN_global_alpha

   #define GL_SUN_global_alpha 1
   #define GL_SUN_global_alpha_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_GLOBAL_ALPHA_SUN

      #define GL_GLOBAL_ALPHA_SUN                                         0x81d9
      #define GL_GLOBAL_ALPHA_FACTOR_SUN                                  0x81da

   #endif /* GL_GLOBAL_ALPHA_SUN */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glGlobalAlphaFactorbSUN                          OGLEXT_MAKEGLNAME(GlobalAlphaFactorbSUN)
      #define glGlobalAlphaFactordSUN                          OGLEXT_MAKEGLNAME(GlobalAlphaFactordSUN)
      #define glGlobalAlphaFactorfSUN                          OGLEXT_MAKEGLNAME(GlobalAlphaFactorfSUN)
      #define glGlobalAlphaFactoriSUN                          OGLEXT_MAKEGLNAME(GlobalAlphaFactoriSUN)
      #define glGlobalAlphaFactorsSUN                          OGLEXT_MAKEGLNAME(GlobalAlphaFactorsSUN)
      #define glGlobalAlphaFactorubSUN                         OGLEXT_MAKEGLNAME(GlobalAlphaFactorubSUN)
      #define glGlobalAlphaFactoruiSUN                         OGLEXT_MAKEGLNAME(GlobalAlphaFactoruiSUN)
      #define glGlobalAlphaFactorusSUN                         OGLEXT_MAKEGLNAME(GlobalAlphaFactorusSUN)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glGlobalAlphaFactorbSUN(GLbyte);
      GLAPI GLvoid            glGlobalAlphaFactordSUN(GLdouble);
      GLAPI GLvoid            glGlobalAlphaFactorfSUN(GLfloat);
      GLAPI GLvoid            glGlobalAlphaFactoriSUN(GLint);
      GLAPI GLvoid            glGlobalAlphaFactorsSUN(GLshort);
      GLAPI GLvoid            glGlobalAlphaFactorubSUN(GLubyte);
      GLAPI GLvoid            glGlobalAlphaFactoruiSUN(GLuint);
      GLAPI GLvoid            glGlobalAlphaFactorusSUN(GLushort);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SUN_global_alpha */


/* ---[ GL_SUN_mesh_array ]---------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SUN/mesh_array.txt */

#ifndef GL_SUN_mesh_array

   #define GL_SUN_mesh_array 1
   #define GL_SUN_mesh_array_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_QUAD_MESH_SUN

      #define GL_QUAD_MESH_SUN                                            0x8614
      #define GL_TRIANGLE_MESH_SUN                                        0x8615

   #endif /* GL_QUAD_MESH_SUN */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glDrawMeshArraysSUN                              OGLEXT_MAKEGLNAME(DrawMeshArraysSUN)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glDrawMeshArraysSUN(GLenum, GLint, GLsizei, GLsizei);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SUN_mesh_array */


/* ---[ GL_SUN_slice_accum ]--------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SUN/slice_accum.txt */

#ifndef GL_SUN_slice_accum

   #define GL_SUN_slice_accum 1
   #define GL_SUN_slice_accum_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_SLICE_ACCUM_SUN

      #define GL_SLICE_ACCUM_SUN                                          0x85cc

   #endif /* GL_SLICE_ACCUM_SUN */

#endif /* GL_SUN_slice_accum */


/* ---[ GL_SUN_triangle_list ]------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SUN/triangle_list.txt */

#ifndef GL_SUN_triangle_list

   #define GL_SUN_triangle_list 1
   #define GL_SUN_triangle_list_OGLEXT 1

   /* - -[ constants ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef GL_RESTART_SUN

      #define GL_RESTART_SUN                                              0x1
      #define GL_REPLACE_MIDDLE_SUN                                       0x2
      #define GL_REPLACE_OLDEST_SUN                                       0x3
      #define GL_TRIANGLE_LIST_SUN                                        0x81d7
      #define GL_REPLACEMENT_CODE_SUN                                     0x81d8
      #define GL_REPLACEMENT_CODE_ARRAY_SUN                               0x85c0
      #define GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN                          0x85c1
      #define GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN                        0x85c2
      #define GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN                       0x85c3
      #define GL_R1UI_V3F_SUN                                             0x85c4
      #define GL_R1UI_C4UB_V3F_SUN                                        0x85c5
      #define GL_R1UI_C3F_V3F_SUN                                         0x85c6
      #define GL_R1UI_N3F_V3F_SUN                                         0x85c7
      #define GL_R1UI_C4F_N3F_V3F_SUN                                     0x85c8
      #define GL_R1UI_T2F_V3F_SUN                                         0x85c9
      #define GL_R1UI_T2F_N3F_V3F_SUN                                     0x85ca
      #define GL_R1UI_T2F_C4F_N3F_V3F_SUN                                 0x85cb

   #endif /* GL_RESTART_SUN */

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glReplacementCodePointerSUN                      OGLEXT_MAKEGLNAME(ReplacementCodePointerSUN)
      #define glReplacementCodeubSUN                           OGLEXT_MAKEGLNAME(ReplacementCodeubSUN)
      #define glReplacementCodeubvSUN                          OGLEXT_MAKEGLNAME(ReplacementCodeubvSUN)
      #define glReplacementCodeuiSUN                           OGLEXT_MAKEGLNAME(ReplacementCodeuiSUN)
      #define glReplacementCodeuivSUN                          OGLEXT_MAKEGLNAME(ReplacementCodeuivSUN)
      #define glReplacementCodeusSUN                           OGLEXT_MAKEGLNAME(ReplacementCodeusSUN)
      #define glReplacementCodeusvSUN                          OGLEXT_MAKEGLNAME(ReplacementCodeusvSUN)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glReplacementCodePointerSUN(GLenum, GLsizei, GLvoid const * *);
      GLAPI GLvoid            glReplacementCodeubSUN(GLubyte);
      GLAPI GLvoid            glReplacementCodeubvSUN(GLubyte const *);
      GLAPI GLvoid            glReplacementCodeuiSUN(GLuint);
      GLAPI GLvoid            glReplacementCodeuivSUN(GLuint const *);
      GLAPI GLvoid            glReplacementCodeusSUN(GLushort);
      GLAPI GLvoid            glReplacementCodeusvSUN(GLushort const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SUN_triangle_list */


/* ---[ GL_SUN_vertex ]-------------------------------------------------------------------------------------- */

/*! link : http://oss.sgi.com/projects/ogl-sample/registry/SUN/vertex.txt */

#ifndef GL_SUN_vertex

   #define GL_SUN_vertex 1
   #define GL_SUN_vertex_OGLEXT 1

   /* - -[ function name definitions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifndef OGLGLEXT_NOALIAS

      #define glColor3fVertex3fSUN                             OGLEXT_MAKEGLNAME(Color3fVertex3fSUN)
      #define glColor3fVertex3fvSUN                            OGLEXT_MAKEGLNAME(Color3fVertex3fvSUN)
      #define glColor4fNormal3fVertex3fSUN                     OGLEXT_MAKEGLNAME(Color4fNormal3fVertex3fSUN)
      #define glColor4fNormal3fVertex3fvSUN                    OGLEXT_MAKEGLNAME(Color4fNormal3fVertex3fvSUN)
      #define glColor4ubVertex2fSUN                            OGLEXT_MAKEGLNAME(Color4ubVertex2fSUN)
      #define glColor4ubVertex2fvSUN                           OGLEXT_MAKEGLNAME(Color4ubVertex2fvSUN)
      #define glColor4ubVertex3fSUN                            OGLEXT_MAKEGLNAME(Color4ubVertex3fSUN)
      #define glColor4ubVertex3fvSUN                           OGLEXT_MAKEGLNAME(Color4ubVertex3fvSUN)
      #define glNormal3fVertex3fSUN                            OGLEXT_MAKEGLNAME(Normal3fVertex3fSUN)
      #define glNormal3fVertex3fvSUN                           OGLEXT_MAKEGLNAME(Normal3fVertex3fvSUN)
      #define glReplacementCodeuiColor3fVertex3fSUN            OGLEXT_MAKEGLNAME(ReplacementCodeuiColor3fVertex3fSUN)
      #define glReplacementCodeuiColor3fVertex3fvSUN           OGLEXT_MAKEGLNAME(ReplacementCodeuiColor3fVertex3fvSUN)
      #define glReplacementCodeuiColor4fNormal3fVertex3fSUN    OGLEXT_MAKEGLNAME(ReplacementCodeuiColor4fNormal3fVertex3fSUN)
      #define glReplacementCodeuiColor4fNormal3fVertex3fvSUN   OGLEXT_MAKEGLNAME(ReplacementCodeuiColor4fNormal3fVertex3fvSUN)
      #define glReplacementCodeuiColor4ubVertex3fSUN           OGLEXT_MAKEGLNAME(ReplacementCodeuiColor4ubVertex3fSUN)
      #define glReplacementCodeuiColor4ubVertex3fvSUN          OGLEXT_MAKEGLNAME(ReplacementCodeuiColor4ubVertex3fvSUN)
      #define glReplacementCodeuiNormal3fVertex3fSUN           OGLEXT_MAKEGLNAME(ReplacementCodeuiNormal3fVertex3fSUN)
      #define glReplacementCodeuiNormal3fVertex3fvSUN          OGLEXT_MAKEGLNAME(ReplacementCodeuiNormal3fVertex3fvSUN)
      #define glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN OGLEXT_MAKEGLNAME(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN)
      #define glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN OGLEXT_MAKEGLNAME(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN)
      #define glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN OGLEXT_MAKEGLNAME(ReplacementCodeuiTexCoord2fNormal3fVertex3fSUN)
      #define glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN OGLEXT_MAKEGLNAME(ReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN)
      #define glReplacementCodeuiTexCoord2fVertex3fSUN         OGLEXT_MAKEGLNAME(ReplacementCodeuiTexCoord2fVertex3fSUN)
      #define glReplacementCodeuiTexCoord2fVertex3fvSUN        OGLEXT_MAKEGLNAME(ReplacementCodeuiTexCoord2fVertex3fvSUN)
      #define glReplacementCodeuiVertex3fSUN                   OGLEXT_MAKEGLNAME(ReplacementCodeuiVertex3fSUN)
      #define glReplacementCodeuiVertex3fvSUN                  OGLEXT_MAKEGLNAME(ReplacementCodeuiVertex3fvSUN)
      #define glTexCoord2fColor3fVertex3fSUN                   OGLEXT_MAKEGLNAME(TexCoord2fColor3fVertex3fSUN)
      #define glTexCoord2fColor3fVertex3fvSUN                  OGLEXT_MAKEGLNAME(TexCoord2fColor3fVertex3fvSUN)
      #define glTexCoord2fColor4fNormal3fVertex3fSUN           OGLEXT_MAKEGLNAME(TexCoord2fColor4fNormal3fVertex3fSUN)
      #define glTexCoord2fColor4fNormal3fVertex3fvSUN          OGLEXT_MAKEGLNAME(TexCoord2fColor4fNormal3fVertex3fvSUN)
      #define glTexCoord2fColor4ubVertex3fSUN                  OGLEXT_MAKEGLNAME(TexCoord2fColor4ubVertex3fSUN)
      #define glTexCoord2fColor4ubVertex3fvSUN                 OGLEXT_MAKEGLNAME(TexCoord2fColor4ubVertex3fvSUN)
      #define glTexCoord2fNormal3fVertex3fSUN                  OGLEXT_MAKEGLNAME(TexCoord2fNormal3fVertex3fSUN)
      #define glTexCoord2fNormal3fVertex3fvSUN                 OGLEXT_MAKEGLNAME(TexCoord2fNormal3fVertex3fvSUN)
      #define glTexCoord2fVertex3fSUN                          OGLEXT_MAKEGLNAME(TexCoord2fVertex3fSUN)
      #define glTexCoord2fVertex3fvSUN                         OGLEXT_MAKEGLNAME(TexCoord2fVertex3fvSUN)
      #define glTexCoord4fColor4fNormal3fVertex4fSUN           OGLEXT_MAKEGLNAME(TexCoord4fColor4fNormal3fVertex4fSUN)
      #define glTexCoord4fColor4fNormal3fVertex4fvSUN          OGLEXT_MAKEGLNAME(TexCoord4fColor4fNormal3fVertex4fvSUN)
      #define glTexCoord4fVertex4fSUN                          OGLEXT_MAKEGLNAME(TexCoord4fVertex4fSUN)
      #define glTexCoord4fVertex4fvSUN                         OGLEXT_MAKEGLNAME(TexCoord4fVertex4fvSUN)

   #endif /* OGLGLEXT_NOALIAS */

   /* - -[ function types ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   #ifdef GL_GLEXT_PROTOTYPES

      GLAPI GLvoid            glColor3fVertex3fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glColor3fVertex3fvSUN(GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glColor4fNormal3fVertex3fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glColor4fNormal3fVertex3fvSUN(GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glColor4ubVertex2fSUN(GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat);
      GLAPI GLvoid            glColor4ubVertex2fvSUN(GLubyte const *, GLfloat const *);
      GLAPI GLvoid            glColor4ubVertex3fSUN(GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glColor4ubVertex3fvSUN(GLubyte const *, GLfloat const *);
      GLAPI GLvoid            glNormal3fVertex3fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glNormal3fVertex3fvSUN(GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiColor3fVertex3fSUN(GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiColor3fVertex3fvSUN(GLuint const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiColor4fNormal3fVertex3fSUN(GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiColor4fNormal3fVertex3fvSUN(GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiColor4ubVertex3fSUN(GLuint, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiColor4ubVertex3fvSUN(GLuint const *, GLubyte const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiNormal3fVertex3fSUN(GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiNormal3fVertex3fvSUN(GLuint const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN(GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN(GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiTexCoord2fVertex3fSUN(GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiTexCoord2fVertex3fvSUN(GLuint const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glReplacementCodeuiVertex3fSUN(GLuint, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glReplacementCodeuiVertex3fvSUN(GLuint const *, GLfloat const *);
      GLAPI GLvoid            glTexCoord2fColor3fVertex3fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTexCoord2fColor3fVertex3fvSUN(GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glTexCoord2fColor4fNormal3fVertex3fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTexCoord2fColor4fNormal3fVertex3fvSUN(GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glTexCoord2fColor4ubVertex3fSUN(GLfloat, GLfloat, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTexCoord2fColor4ubVertex3fvSUN(GLfloat const *, GLubyte const *, GLfloat const *);
      GLAPI GLvoid            glTexCoord2fNormal3fVertex3fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTexCoord2fNormal3fVertex3fvSUN(GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glTexCoord2fVertex3fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTexCoord2fVertex3fvSUN(GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glTexCoord4fColor4fNormal3fVertex4fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTexCoord4fColor4fNormal3fVertex4fvSUN(GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *);
      GLAPI GLvoid            glTexCoord4fVertex4fSUN(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
      GLAPI GLvoid            glTexCoord4fVertex4fvSUN(GLfloat const *, GLfloat const *);

   #endif /* GL_GLEXT_PROTOTYPES */

#endif /* GL_SUN_vertex */


#ifdef __cplusplus
}
#endif

#endif	/* _GLEXT_H_ */
