// RenderingContext.hpp                                      Copyright (C) 2006 Thomas Jansen (jansen@caesar.de)
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

#ifndef	_OGL_RENDERINGCONTEXT_HPP_
#define	_OGL_RENDERINGCONTEXT_HPP_

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
	#define	WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
#endif

#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
	#include <mach-o/dyld.h>
#endif

#include	<stdlib.h>
#include	<string.h>

#include	"glext.h"


// ---[ CLASS PROTO-TYPES ]-------------------------------------------------------------------------------------

class		CRCHashArray;


// ---[ CLASS DECLARATION ]-------------------------------------------------------------------------------------

//!	The rendering context specific API entry point object.

class CRenderingContext {

	friend	class					CRCHashArray;

	// - -[ structures ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	//!	List node for extension names.

	struct SExtensionName {

		SExtensionName *			pNext;													//!<       The next extension string.
		char const *				szExtension;											//!<              The extension name.
	};

protected:

	// - -[ class administration ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

										CRenderingContext();									//                    the constructor
									  ~CRenderingContext();									//                     the destructor

	// - -[ tool methods ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void								InitExtensionString();								//    initialize the extension string
	void								InitVersionString();									//      initialize the version string

	// - -[ object administration ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool								InitVersion12();                             //                     GL_VERSION_1_2
	bool								InitVersion13();                             //                     GL_VERSION_1_3
	bool								InitVersion14();                             //                     GL_VERSION_1_4
	bool								InitVersion15();                             //                     GL_VERSION_1_5
	bool								InitVersion20();                             //                     GL_VERSION_2_0

	bool								Init3dfxTbuffer();                           //                    GL_3DFX_tbuffer
	bool								InitAppleElementArray();                     //             GL_APPLE_element_array
	bool								InitAppleFence();                            //                     GL_APPLE_fence
	bool								InitAppleVertexArrayObject();                //       GL_APPLE_vertex_array_object
	bool								InitAppleVertexArrayRange();                 //        GL_APPLE_vertex_array_range
	bool								InitArbColorBufferFloat();                   //          GL_ARB_color_buffer_float
	bool								InitArbDrawBuffers();                        //                GL_ARB_draw_buffers
	bool								InitArbMatrixPalette();                      //              GL_ARB_matrix_palette
	bool								InitArbMultisample();                        //                 GL_ARB_multisample
	bool								InitArbMultitexture();                       //                GL_ARB_multitexture
	bool								InitArbOcclusionQuery();                     //             GL_ARB_occlusion_query
	bool								InitArbPointParameters();                    //            GL_ARB_point_parameters
	bool								InitArbShaderObjects();                      //              GL_ARB_shader_objects
	bool								InitArbTextureCompression();                 //         GL_ARB_texture_compression
	bool								InitArbTransposeMatrix();                    //            GL_ARB_transpose_matrix
	bool								InitArbVertexBlend();                        //                GL_ARB_vertex_blend
	bool								InitArbVertexBufferObject();                 //        GL_ARB_vertex_buffer_object
	bool								InitArbVertexProgram();                      //              GL_ARB_vertex_program
	bool								InitArbVertexShader();                       //               GL_ARB_vertex_shader
	bool								InitArbWindowPos();                          //                  GL_ARB_window_pos
	bool								InitAtiDrawBuffers();                        //                GL_ATI_draw_buffers
	bool								InitAtiElementArray();                       //               GL_ATI_element_array
	bool								InitAtiEnvmapBumpmap();                      //              GL_ATI_envmap_bumpmap
	bool								InitAtiFragmentShader();                     //             GL_ATI_fragment_shader
	bool								InitAtiMapObjectBuffer();                    //           GL_ATI_map_object_buffer
	bool								InitAtiPnTriangles();                        //                GL_ATI_pn_triangles
	bool								InitAtiSeparateStencil();                    //            GL_ATI_separate_stencil
	bool								InitAtiVertexArrayObject();                  //         GL_ATI_vertex_array_object
	bool								InitAtiVertexAttribArrayObject();            //  GL_ATI_vertex_attrib_array_object
	bool								InitAtiVertexStreams();                      //              GL_ATI_vertex_streams
	bool								InitExtBlendColor();                         //                 GL_EXT_blend_color
	bool								InitExtBlendEquationSeparate();              //     GL_EXT_blend_equation_separate
	bool								InitExtBlendFuncSeparate();                  //         GL_EXT_blend_func_separate
	bool								InitExtBlendMinmax();                        //                GL_EXT_blend_minmax
	bool								InitExtColorSubtable();                      //              GL_EXT_color_subtable
	bool								InitExtCompiledVertexArray();                //       GL_EXT_compiled_vertex_array
	bool								InitExtConvolution();                        //                 GL_EXT_convolution
	bool								InitExtCoordinateFrame();                    //            GL_EXT_coordinate_frame
	bool								InitExtCopyTexture();                        //                GL_EXT_copy_texture
	bool								InitExtCullVertex();                         //                 GL_EXT_cull_vertex
	bool								InitExtDepthBoundsTest();                    //           GL_EXT_depth_bounds_test
	bool								InitExtDrawRangeElements();                  //         GL_EXT_draw_range_elements
	bool								InitExtFogCoord();                           //                   GL_EXT_fog_coord
	bool								InitExtFramebufferObject();                  //          GL_EXT_framebuffer_object
	bool								InitExtHistogram();                          //                   GL_EXT_histogram
	bool								InitExtIndexFunc();                          //                  GL_EXT_index_func
	bool								InitExtIndexMaterial();                      //              GL_EXT_index_material
	bool								InitExtLightTexture();                       //               GL_EXT_light_texture
	bool								InitExtMultiDrawArrays();                    //           GL_EXT_multi_draw_arrays
	bool								InitExtMultisample();                        //                 GL_EXT_multisample
	bool								InitExtPalettedTexture();                    //            GL_EXT_paletted_texture
	bool								InitExtPixelTransform();                     //             GL_EXT_pixel_transform
	bool								InitExtPointParameters();                    //            GL_EXT_point_parameters
	bool								InitExtPolygonOffset();                      //              GL_EXT_polygon_offset
	bool								InitExtSecondaryColor();                     //             GL_EXT_secondary_color
	bool								InitExtStencilTwoSide();                     //            GL_EXT_stencil_two_side
	bool								InitExtSubtexture();                         //                  GL_EXT_subtexture
	bool								InitExtTexture3d();                          //                   GL_EXT_texture3D
	bool								InitExtTextureObject();                      //              GL_EXT_texture_object
	bool								InitExtTexturePerturbNormal();               //      GL_EXT_texture_perturb_normal
	bool								InitExtVertexArray();                        //                GL_EXT_vertex_array
	bool								InitExtVertexShader();                       //               GL_EXT_vertex_shader
	bool								InitExtVertexWeighting();                    //            GL_EXT_vertex_weighting
	bool								InitGremedyStringMarker();                   //           GL_GREMEDY_string_marker
	bool								InitHpImageTransform();                      //              GL_HP_image_transform
	bool								InitIbmMultimodeDrawArrays();                //       GL_IBM_multimode_draw_arrays
	bool								InitIbmVertexArrayLists();                   //          GL_IBM_vertex_array_lists
	bool								InitIngrBlendFuncSeparate();                 //        GL_INGR_blend_func_separate
	bool								InitIntelParallelArrays();                   //           GL_INTEL_parallel_arrays
	bool								InitMesaResizeBuffers();                     //             GL_MESA_resize_buffers
	bool								InitMesaWindowPos();                         //                 GL_MESA_window_pos
	bool								InitNvElementArray();                        //                GL_NV_element_array
	bool								InitNvEvaluators();                          //                   GL_NV_evaluators
	bool								InitNvFence();                               //                        GL_NV_fence
	bool								InitNvFragmentProgram();                     //             GL_NV_fragment_program
	bool								InitNvHalfFloat();                           //                   GL_NV_half_float
	bool								InitNvOcclusionQuery();                      //              GL_NV_occlusion_query
	bool								InitNvPixelDataRange();                      //             GL_NV_pixel_data_range
	bool								InitNvPointSprite();                         //                 GL_NV_point_sprite
	bool								InitNvPrimitiveRestart();                    //            GL_NV_primitive_restart
	bool								InitNvRegisterCombiners();                   //           GL_NV_register_combiners
	bool								InitNvRegisterCombiners2();                  //          GL_NV_register_combiners2
	bool								InitNvStencilTwoSide();                      //             GL_NV_stencil_two_side
	bool								InitNvVertexArrayRange();                    //           GL_NV_vertex_array_range
	bool								InitNvVertexProgram();                       //               GL_NV_vertex_program
	bool								InitNvxConditionalRender();                  //          GL_NVX_conditional_render
	bool								InitPgiMiscHints();                          //                  GL_PGI_misc_hints
	bool								InitSgiColorTable();                         //                 GL_SGI_color_table
	bool								InitSgisDetailTexture();                     //             GL_SGIS_detail_texture
	bool								InitSgisFogFunction();                       //               GL_SGIS_fog_function
	bool								InitSgisMultisample();                       //                GL_SGIS_multisample
	bool								InitSgisPixelTexture();                      //              GL_SGIS_pixel_texture
	bool								InitSgisPointParameters();                   //           GL_SGIS_point_parameters
	bool								InitSgisSharpenTexture();                    //            GL_SGIS_sharpen_texture
	bool								InitSgisTexture4d();                         //                  GL_SGIS_texture4D
	bool								InitSgisTextureColorMask();                  //         GL_SGIS_texture_color_mask
	bool								InitSgisTextureFilter4();                    //            GL_SGIS_texture_filter4
	bool								InitSgixAsync();                             //                      GL_SGIX_async
	bool								InitSgixFlushRaster();                       //               GL_SGIX_flush_raster
	bool								InitSgixFragmentLighting();                  //          GL_SGIX_fragment_lighting
	bool								InitSgixFramezoom();                         //                  GL_SGIX_framezoom
	bool								InitSgixIglooInterface();                    //            GL_SGIX_igloo_interface
	bool								InitSgixInstruments();                       //                GL_SGIX_instruments
	bool								InitSgixListPriority();                      //              GL_SGIX_list_priority
	bool								InitSgixPixelTexture();                      //              GL_SGIX_pixel_texture
	bool								InitSgixPolynomialFfd();                     //             GL_SGIX_polynomial_ffd
	bool								InitSgixReferencePlane();                    //            GL_SGIX_reference_plane
	bool								InitSgixSprite();                            //                     GL_SGIX_sprite
	bool								InitSgixTagSampleBuffer();                   //          GL_SGIX_tag_sample_buffer
	bool								InitSunGlobalAlpha();                        //                GL_SUN_global_alpha
	bool								InitSunMeshArray();                          //                  GL_SUN_mesh_array
	bool								InitSunTriangleList();                       //               GL_SUN_triangle_list
	bool								InitSunVertex();                             //                      GL_SUN_vertex
	bool								InitSunxConstantData();                      //              GL_SUNX_constant_data

	// - -[ variables ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	SExtensionName *				m_pFirstExtensionName;								//!<            First extension name.
	unsigned long					m_uVersion;												//!<              The OpenGL version.
	
public:

	// - -[ tool methods ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool								IsExtensionSupported(const char * szExtensions) const;	//  extensions supported?
	unsigned long					GetVersion() const;													//     return the version

	// - -[ static functions ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static	void *				GetProcAddress(char const * szFunction);		//    cross-platform GetProcAddress()

	// - -[ gl_version_1_2 ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_VERSION_1_2_OGLEXT

		GLvoid            (APIENTRY * m_pBlendColor) (GLclampf, GLclampf, GLclampf, GLclampf);
		GLvoid            (APIENTRY * m_pBlendEquation) (GLenum);
		GLvoid            (APIENTRY * m_pColorSubTable) (GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pColorTable) (GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pColorTableParameterfv) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pColorTableParameteriv) (GLenum, GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pConvolutionFilter1D) (GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pConvolutionFilter2D) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pConvolutionParameterf) (GLenum, GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pConvolutionParameterfv) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pConvolutionParameteri) (GLenum, GLenum, GLint);
		GLvoid            (APIENTRY * m_pConvolutionParameteriv) (GLenum, GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pCopyColorSubTable) (GLenum, GLsizei, GLint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pCopyColorTable) (GLenum, GLenum, GLint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pCopyConvolutionFilter1D) (GLenum, GLenum, GLint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pCopyConvolutionFilter2D) (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
		GLvoid            (APIENTRY * m_pCopyTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
		GLvoid            (APIENTRY * m_pDrawRangeElements) (GLenum, GLuint, GLuint, GLsizei, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pGetColorTable) (GLenum, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetColorTableParameterfv) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetColorTableParameteriv) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetConvolutionFilter) (GLenum, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetConvolutionParameterfv) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetConvolutionParameteriv) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetHistogram) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetHistogramParameterfv) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetHistogramParameteriv) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetMinmax) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetMinmaxParameterfv) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetMinmaxParameteriv) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetSeparableFilter) (GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
		GLvoid            (APIENTRY * m_pHistogram) (GLenum, GLsizei, GLenum, GLboolean);
		GLvoid            (APIENTRY * m_pMinmax) (GLenum, GLenum, GLboolean);
		GLvoid            (APIENTRY * m_pResetHistogram) (GLenum);
		GLvoid            (APIENTRY * m_pResetMinmax) (GLenum);
		GLvoid            (APIENTRY * m_pSeparableFilter2D) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *, GLvoid const *);
		GLvoid            (APIENTRY * m_pTexImage3D) (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

	#endif // GL_VERSION_1_2_OGLEXT

	// - -[ gl_version_1_3 ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_VERSION_1_3_OGLEXT

		GLvoid            (APIENTRY * m_pActiveTexture) (GLenum);
		GLvoid            (APIENTRY * m_pClientActiveTexture) (GLenum);
		GLvoid            (APIENTRY * m_pCompressedTexImage1D) (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexImage2D) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexImage3D) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexSubImage1D) (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexSubImage2D) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexSubImage3D) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pGetCompressedTexImage) (GLenum, GLint, GLvoid *);
		GLvoid            (APIENTRY * m_pLoadTransposeMatrixd) (GLdouble const *);
		GLvoid            (APIENTRY * m_pLoadTransposeMatrixf) (GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1d) (GLenum, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord1dv) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1f) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord1fv) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1i) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord1iv) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1s) (GLenum, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord1sv) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2d) (GLenum, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord2dv) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2f) (GLenum, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord2fv) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2i) (GLenum, GLint, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord2iv) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2s) (GLenum, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord2sv) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3d) (GLenum, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord3dv) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3f) (GLenum, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord3fv) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3i) (GLenum, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord3iv) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3s) (GLenum, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord3sv) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4d) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord4dv) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4f) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord4fv) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4i) (GLenum, GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord4iv) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4s) (GLenum, GLshort, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord4sv) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pMultTransposeMatrixd) (GLdouble const *);
		GLvoid            (APIENTRY * m_pMultTransposeMatrixf) (GLfloat const *);
		GLvoid            (APIENTRY * m_pSampleCoverage) (GLclampf, GLboolean);

	#endif // GL_VERSION_1_3_OGLEXT

	// - -[ gl_version_1_4 ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_VERSION_1_4_OGLEXT

		GLvoid            (APIENTRY * m_pBlendFuncSeparate) (GLenum, GLenum, GLenum, GLenum);
		GLvoid            (APIENTRY * m_pFogCoordd) (GLdouble);
		GLvoid            (APIENTRY * m_pFogCoorddv) (GLdouble const *);
		GLvoid            (APIENTRY * m_pFogCoordf) (GLfloat);
		GLvoid            (APIENTRY * m_pFogCoordfv) (GLfloat const *);
		GLvoid            (APIENTRY * m_pFogCoordPointer) (GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pMultiDrawArrays) (GLenum, GLint *, GLsizei *, GLsizei);
		GLvoid            (APIENTRY * m_pMultiDrawElements) (GLenum, GLsizei const *, GLenum, GLvoid const * *, GLsizei);
		GLvoid            (APIENTRY * m_pPointParameterf) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pPointParameterfv) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pPointParameteri) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pPointParameteriv) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3b) (GLbyte, GLbyte, GLbyte);
		GLvoid            (APIENTRY * m_pSecondaryColor3bv) (GLbyte const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3d) (GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pSecondaryColor3dv) (GLdouble const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3f) (GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pSecondaryColor3fv) (GLfloat const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3i) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pSecondaryColor3iv) (GLint const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3s) (GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pSecondaryColor3sv) (GLshort const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3ub) (GLubyte, GLubyte, GLubyte);
		GLvoid            (APIENTRY * m_pSecondaryColor3ubv) (GLubyte const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3ui) (GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pSecondaryColor3uiv) (GLuint const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3us) (GLushort, GLushort, GLushort);
		GLvoid            (APIENTRY * m_pSecondaryColor3usv) (GLushort const *);
		GLvoid            (APIENTRY * m_pSecondaryColorPointer) (GLint, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pWindowPos2d) (GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pWindowPos2dv) (GLdouble const *);
		GLvoid            (APIENTRY * m_pWindowPos2f) (GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pWindowPos2fv) (GLfloat const *);
		GLvoid            (APIENTRY * m_pWindowPos2i) (GLint, GLint);
		GLvoid            (APIENTRY * m_pWindowPos2iv) (GLint const *);
		GLvoid            (APIENTRY * m_pWindowPos2s) (GLshort, GLshort);
		GLvoid            (APIENTRY * m_pWindowPos2sv) (GLshort const *);
		GLvoid            (APIENTRY * m_pWindowPos3d) (GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pWindowPos3dv) (GLdouble const *);
		GLvoid            (APIENTRY * m_pWindowPos3f) (GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pWindowPos3fv) (GLfloat const *);
		GLvoid            (APIENTRY * m_pWindowPos3i) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pWindowPos3iv) (GLint const *);
		GLvoid            (APIENTRY * m_pWindowPos3s) (GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pWindowPos3sv) (GLshort const *);

	#endif // GL_VERSION_1_4_OGLEXT

	// - -[ gl_version_1_5 ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_VERSION_1_5_OGLEXT

		GLvoid            (APIENTRY * m_pBeginQuery) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pBindBuffer) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pBufferData) (GLenum, GLsizeiptr, GLvoid const *, GLenum);
		GLvoid            (APIENTRY * m_pBufferSubData) (GLenum, GLintptr, GLsizeiptr, GLvoid const *);
		GLvoid            (APIENTRY * m_pDeleteBuffers) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pDeleteQueries) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pEndQuery) (GLenum);
		GLvoid            (APIENTRY * m_pGenBuffers) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGenQueries) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetBufferParameteriv) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetBufferPointerv) (GLenum, GLenum, GLvoid * *);
		GLvoid            (APIENTRY * m_pGetBufferSubData) (GLenum, GLintptr, GLsizeiptr, GLvoid *);
		GLvoid            (APIENTRY * m_pGetQueryiv) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetQueryObjectiv) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetQueryObjectuiv) (GLuint, GLenum, GLuint *);
		GLboolean         (APIENTRY * m_pIsBuffer) (GLuint);
		GLboolean         (APIENTRY * m_pIsQuery) (GLuint);
		GLvoid *          (APIENTRY * m_pMapBuffer) (GLenum, GLenum);
		GLboolean         (APIENTRY * m_pUnmapBuffer) (GLenum);

	#endif // GL_VERSION_1_5_OGLEXT

	// - -[ gl_version_2_0 ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_VERSION_2_0_OGLEXT

		GLvoid            (APIENTRY * m_pAttachShader) (GLuint, GLuint);
		GLvoid            (APIENTRY * m_pBindAttribLocation) (GLuint, GLuint, GLchar const *);
		GLvoid            (APIENTRY * m_pBlendEquationSeparate) (GLenum, GLenum);
		GLvoid            (APIENTRY * m_pCompileShader) (GLuint);
		GLuint            (APIENTRY * m_pCreateProgram) ();
		GLuint            (APIENTRY * m_pCreateShader) (GLenum);
		GLvoid            (APIENTRY * m_pDeleteProgram) (GLuint);
		GLvoid            (APIENTRY * m_pDeleteShader) (GLuint);
		GLvoid            (APIENTRY * m_pDetachShader) (GLuint, GLuint);
		GLvoid            (APIENTRY * m_pDisableVertexAttribArray) (GLuint);
		GLvoid            (APIENTRY * m_pDrawBuffers) (GLsizei, GLenum const *);
		GLvoid            (APIENTRY * m_pEnableVertexAttribArray) (GLuint);
		GLvoid            (APIENTRY * m_pGetActiveAttrib) (GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
		GLvoid            (APIENTRY * m_pGetActiveUniform) (GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
		GLvoid            (APIENTRY * m_pGetAttachedShaders) (GLuint, GLsizei, GLsizei *, GLuint *);
		GLint             (APIENTRY * m_pGetAttribLocation) (GLuint, GLchar const *);
		GLvoid            (APIENTRY * m_pGetProgramInfoLog) (GLuint, GLsizei, GLsizei *, GLchar *);
		GLvoid            (APIENTRY * m_pGetProgramiv) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetShaderInfoLog) (GLuint, GLsizei, GLsizei *, GLchar *);
		GLvoid            (APIENTRY * m_pGetShaderiv) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetShaderSource) (GLuint, GLsizei, GLsizei *, GLchar *);
		GLvoid            (APIENTRY * m_pGetUniformfv) (GLuint, GLint, GLfloat *);
		GLvoid            (APIENTRY * m_pGetUniformiv) (GLuint, GLint, GLint *);
		GLint             (APIENTRY * m_pGetUniformLocation) (GLuint, GLchar const *);
		GLvoid            (APIENTRY * m_pGetVertexAttribdv) (GLuint, GLenum, GLdouble *);
		GLvoid            (APIENTRY * m_pGetVertexAttribfv) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetVertexAttribiv) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetVertexAttribPointerv) (GLuint, GLenum, GLvoid * *);
		GLboolean         (APIENTRY * m_pIsProgram) (GLuint);
		GLboolean         (APIENTRY * m_pIsShader) (GLuint);
		GLvoid            (APIENTRY * m_pLinkProgram) (GLuint);
		GLvoid            (APIENTRY * m_pShaderSource) (GLuint, GLsizei, GLchar const * *, GLint const *);
		GLvoid            (APIENTRY * m_pStencilFuncSeparate) (GLenum, GLenum, GLint, GLuint);
		GLvoid            (APIENTRY * m_pStencilMaskSeparate) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pStencilOpSeparate) (GLenum, GLenum, GLenum, GLenum);
		GLvoid            (APIENTRY * m_pUniform1f) (GLint, GLfloat);
		GLvoid            (APIENTRY * m_pUniform1fv) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform1i) (GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform1iv) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniform2f) (GLint, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pUniform2fv) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform2i) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform2iv) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniform3f) (GLint, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pUniform3fv) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform3i) (GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform3iv) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniform4f) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pUniform4fv) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform4i) (GLint, GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform4iv) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniformMatrix2fv) (GLint, GLsizei, GLboolean, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniformMatrix3fv) (GLint, GLsizei, GLboolean, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniformMatrix4fv) (GLint, GLsizei, GLboolean, GLfloat const *);
		GLvoid            (APIENTRY * m_pUseProgram) (GLuint);
		GLvoid            (APIENTRY * m_pValidateProgram) (GLuint);
		GLvoid            (APIENTRY * m_pVertexAttrib1d) (GLuint, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib1dv) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1f) (GLuint, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib1fv) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1s) (GLuint, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib1sv) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2d) (GLuint, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib2dv) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2f) (GLuint, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib2fv) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2s) (GLuint, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib2sv) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3d) (GLuint, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib3dv) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3f) (GLuint, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib3fv) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3s) (GLuint, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib3sv) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4bv) (GLuint, GLbyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4d) (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib4dv) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4f) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib4fv) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4iv) (GLuint, GLint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4Nbv) (GLuint, GLbyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4Niv) (GLuint, GLint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4Nsv) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4Nub) (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
		GLvoid            (APIENTRY * m_pVertexAttrib4Nubv) (GLuint, GLubyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4Nuiv) (GLuint, GLuint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4Nusv) (GLuint, GLushort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4s) (GLuint, GLshort, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib4sv) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4ubv) (GLuint, GLubyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4uiv) (GLuint, GLuint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4usv) (GLuint, GLushort const *);
		GLvoid            (APIENTRY * m_pVertexAttribPointer) (GLuint, GLint, GLenum, GLboolean, GLsizei, GLvoid const *);

	#endif // GL_VERSION_2_0_OGLEXT

	// - -[ gl_3dfx_tbuffer ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_3DFX_tbuffer_OGLEXT

		GLvoid            (APIENTRY * m_pTbufferMask3DFX) (GLuint);

	#endif // GL_3DFX_tbuffer_OGLEXT

	// - -[ gl_apple_element_array ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_APPLE_element_array_OGLEXT

		GLvoid            (APIENTRY * m_pDrawElementArrayAPPLE) (GLenum, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pDrawRangeElementArrayAPPLE) (GLenum, GLuint, GLuint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pElementPointerAPPLE) (GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pMultiDrawElementArrayAPPLE) (GLenum, GLint const *, GLsizei const *, GLsizei);
		GLvoid            (APIENTRY * m_pMultiDrawRangeElementArrayAPPLE) (GLenum, GLuint, GLuint, GLint const *, GLsizei const *, GLsizei);

	#endif // GL_APPLE_element_array_OGLEXT

	// - -[ gl_apple_fence ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_APPLE_fence_OGLEXT

		GLvoid            (APIENTRY * m_pDeleteFencesAPPLE) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pFinishFenceAPPLE) (GLuint);
		GLvoid            (APIENTRY * m_pFinishObjectAPPLE) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pGenFencesAPPLE) (GLsizei, GLuint *);
		GLboolean         (APIENTRY * m_pIsFenceAPPLE) (GLuint);
		GLvoid            (APIENTRY * m_pSetFenceAPPLE) (GLuint);
		GLboolean         (APIENTRY * m_pTestFenceAPPLE) (GLuint);
		GLboolean         (APIENTRY * m_pTestObjectAPPLE) (GLenum, GLuint);

	#endif // GL_APPLE_fence_OGLEXT

	// - -[ gl_apple_vertex_array_object ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_APPLE_vertex_array_object_OGLEXT

		GLvoid            (APIENTRY * m_pBindVertexArrayAPPLE) (GLuint);
		GLvoid            (APIENTRY * m_pDeleteVertexArraysAPPLE) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pGenVertexArraysAPPLE) (GLsizei, GLuint const *);
		GLboolean         (APIENTRY * m_pIsVertexArrayAPPLE) (GLuint);

	#endif // GL_APPLE_vertex_array_object_OGLEXT

	// - -[ gl_apple_vertex_array_range ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_APPLE_vertex_array_range_OGLEXT

		GLvoid            (APIENTRY * m_pFlushVertexArrayRangeAPPLE) (GLsizei, GLvoid *);
		GLvoid            (APIENTRY * m_pVertexArrayParameteriAPPLE) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pVertexArrayRangeAPPLE) (GLsizei, GLvoid *);

	#endif // GL_APPLE_vertex_array_range_OGLEXT

	// - -[ gl_arb_color_buffer_float ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_color_buffer_float_OGLEXT

		GLvoid            (APIENTRY * m_pClampColorARB) (GLenum, GLenum);

	#endif // GL_ARB_color_buffer_float_OGLEXT

	// - -[ gl_arb_draw_buffers ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_draw_buffers_OGLEXT

		GLvoid            (APIENTRY * m_pDrawBuffersARB) (GLsizei, GLenum const *);

	#endif // GL_ARB_draw_buffers_OGLEXT

	// - -[ gl_arb_matrix_palette ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_matrix_palette_OGLEXT

		GLvoid            (APIENTRY * m_pCurrentPaletteMatrixARB) (GLint);
		GLvoid            (APIENTRY * m_pMatrixIndexPointerARB) (GLint, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pMatrixIndexubvARB) (GLint, GLubyte const *);
		GLvoid            (APIENTRY * m_pMatrixIndexuivARB) (GLint, GLuint const *);
		GLvoid            (APIENTRY * m_pMatrixIndexusvARB) (GLint, GLushort const *);

	#endif // GL_ARB_matrix_palette_OGLEXT

	// - -[ gl_arb_multisample ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_multisample_OGLEXT

		GLvoid            (APIENTRY * m_pSampleCoverageARB) (GLclampf, GLboolean);

	#endif // GL_ARB_multisample_OGLEXT

	// - -[ gl_arb_multitexture ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_multitexture_OGLEXT

		GLvoid            (APIENTRY * m_pActiveTextureARB) (GLenum);
		GLvoid            (APIENTRY * m_pClientActiveTextureARB) (GLenum);
		GLvoid            (APIENTRY * m_pMultiTexCoord1dARB) (GLenum, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord1dvARB) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1fARB) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord1fvARB) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1iARB) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord1ivARB) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1sARB) (GLenum, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord1svARB) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2dARB) (GLenum, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord2dvARB) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2fARB) (GLenum, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord2fvARB) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2iARB) (GLenum, GLint, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord2ivARB) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2sARB) (GLenum, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord2svARB) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3dARB) (GLenum, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord3dvARB) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3fARB) (GLenum, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord3fvARB) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3iARB) (GLenum, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord3ivARB) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3sARB) (GLenum, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord3svARB) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4dARB) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pMultiTexCoord4dvARB) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4fARB) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pMultiTexCoord4fvARB) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4iARB) (GLenum, GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pMultiTexCoord4ivARB) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4sARB) (GLenum, GLshort, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pMultiTexCoord4svARB) (GLenum, GLshort const *);

	#endif // GL_ARB_multitexture_OGLEXT

	// - -[ gl_arb_occlusion_query ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_occlusion_query_OGLEXT

		GLvoid            (APIENTRY * m_pBeginQueryARB) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pDeleteQueriesARB) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pEndQueryARB) (GLenum);
		GLvoid            (APIENTRY * m_pGenQueriesARB) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetQueryivARB) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetQueryObjectivARB) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetQueryObjectuivARB) (GLuint, GLenum, GLuint *);
		GLboolean         (APIENTRY * m_pIsQueryARB) (GLuint);

	#endif // GL_ARB_occlusion_query_OGLEXT

	// - -[ gl_arb_point_parameters ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_point_parameters_OGLEXT

		GLvoid            (APIENTRY * m_pPointParameterfARB) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pPointParameterfvARB) (GLenum, GLfloat const *);

	#endif // GL_ARB_point_parameters_OGLEXT

	// - -[ gl_arb_shader_objects ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_shader_objects_OGLEXT

		GLvoid            (APIENTRY * m_pAttachObjectARB) (GLhandleARB, GLhandleARB);
		GLvoid            (APIENTRY * m_pCompileShaderARB) (GLhandleARB);
		GLhandleARB       (APIENTRY * m_pCreateProgramObjectARB) ();
		GLhandleARB       (APIENTRY * m_pCreateShaderObjectARB) (GLenum);
		GLvoid            (APIENTRY * m_pDeleteObjectARB) (GLhandleARB);
		GLvoid            (APIENTRY * m_pDetachObjectARB) (GLhandleARB, GLhandleARB);
		GLvoid            (APIENTRY * m_pGetActiveUniformARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
		GLvoid            (APIENTRY * m_pGetAttachedObjectsARB) (GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
		GLhandleARB       (APIENTRY * m_pGetHandleARB) (GLenum);
		GLvoid            (APIENTRY * m_pGetInfoLogARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
		GLvoid            (APIENTRY * m_pGetObjectParameterfvARB) (GLhandleARB, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetObjectParameterivARB) (GLhandleARB, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetShaderSourceARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
		GLvoid            (APIENTRY * m_pGetUniformfvARB) (GLhandleARB, GLint, GLfloat *);
		GLvoid            (APIENTRY * m_pGetUniformivARB) (GLhandleARB, GLint, GLint *);
		GLint             (APIENTRY * m_pGetUniformLocationARB) (GLhandleARB, GLcharARB const *);
		GLvoid            (APIENTRY * m_pLinkProgramARB) (GLhandleARB);
		GLvoid            (APIENTRY * m_pShaderSourceARB) (GLhandleARB, GLsizei, GLcharARB const * *, GLint const *);
		GLvoid            (APIENTRY * m_pUniform1fARB) (GLint, GLfloat);
		GLvoid            (APIENTRY * m_pUniform1fvARB) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform1iARB) (GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform1ivARB) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniform2fARB) (GLint, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pUniform2fvARB) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform2iARB) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform2ivARB) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniform3fARB) (GLint, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pUniform3fvARB) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform3iARB) (GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform3ivARB) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniform4fARB) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pUniform4fvARB) (GLint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniform4iARB) (GLint, GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pUniform4ivARB) (GLint, GLsizei, GLint const *);
		GLvoid            (APIENTRY * m_pUniformMatrix2fvARB) (GLint, GLsizei, GLboolean, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniformMatrix3fvARB) (GLint, GLsizei, GLboolean, GLfloat const *);
		GLvoid            (APIENTRY * m_pUniformMatrix4fvARB) (GLint, GLsizei, GLboolean, GLfloat const *);
		GLvoid            (APIENTRY * m_pUseProgramObjectARB) (GLhandleARB);
		GLvoid            (APIENTRY * m_pValidateProgramARB) (GLhandleARB);

	#endif // GL_ARB_shader_objects_OGLEXT

	// - -[ gl_arb_texture_compression ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_texture_compression_OGLEXT

		GLvoid            (APIENTRY * m_pCompressedTexImage1DARB) (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexImage2DARB) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexImage3DARB) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexSubImage1DARB) (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexSubImage2DARB) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pCompressedTexSubImage3DARB) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pGetCompressedTexImageARB) (GLenum, GLint, GLvoid *);

	#endif // GL_ARB_texture_compression_OGLEXT

	// - -[ gl_arb_transpose_matrix ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_transpose_matrix_OGLEXT

		GLvoid            (APIENTRY * m_pLoadTransposeMatrixdARB) (GLdouble const *);
		GLvoid            (APIENTRY * m_pLoadTransposeMatrixfARB) (GLfloat const *);
		GLvoid            (APIENTRY * m_pMultTransposeMatrixdARB) (GLdouble const *);
		GLvoid            (APIENTRY * m_pMultTransposeMatrixfARB) (GLfloat const *);

	#endif // GL_ARB_transpose_matrix_OGLEXT

	// - -[ gl_arb_vertex_blend ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_vertex_blend_OGLEXT

		GLvoid            (APIENTRY * m_pVertexBlendARB) (GLint);
		GLvoid            (APIENTRY * m_pWeightbvARB) (GLint, GLbyte const *);
		GLvoid            (APIENTRY * m_pWeightdvARB) (GLint, GLdouble const *);
		GLvoid            (APIENTRY * m_pWeightfvARB) (GLint, GLfloat const *);
		GLvoid            (APIENTRY * m_pWeightivARB) (GLint, GLint const *);
		GLvoid            (APIENTRY * m_pWeightPointerARB) (GLint, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pWeightsvARB) (GLint, GLshort const *);
		GLvoid            (APIENTRY * m_pWeightubvARB) (GLint, GLubyte const *);
		GLvoid            (APIENTRY * m_pWeightuivARB) (GLint, GLuint const *);
		GLvoid            (APIENTRY * m_pWeightusvARB) (GLint, GLushort const *);

	#endif // GL_ARB_vertex_blend_OGLEXT

	// - -[ gl_arb_vertex_buffer_object ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_vertex_buffer_object_OGLEXT

		GLvoid            (APIENTRY * m_pBindBufferARB) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pBufferDataARB) (GLenum, GLsizeiptrARB, GLvoid const *, GLenum);
		GLvoid            (APIENTRY * m_pBufferSubDataARB) (GLenum, GLintptrARB, GLsizeiptrARB, GLvoid const *);
		GLvoid            (APIENTRY * m_pDeleteBuffersARB) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pGenBuffersARB) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetBufferParameterivARB) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetBufferPointervARB) (GLenum, GLenum, GLvoid * *);
		GLvoid            (APIENTRY * m_pGetBufferSubDataARB) (GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *);
		GLboolean         (APIENTRY * m_pIsBufferARB) (GLuint);
		GLvoid *          (APIENTRY * m_pMapBufferARB) (GLenum, GLenum);
		GLboolean         (APIENTRY * m_pUnmapBufferARB) (GLenum);

	#endif // GL_ARB_vertex_buffer_object_OGLEXT

	// - -[ gl_arb_vertex_program ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_vertex_program_OGLEXT

		GLvoid            (APIENTRY * m_pBindProgramARB) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pDeleteProgramsARB) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pDisableVertexAttribArrayARB) (GLuint);
		GLvoid            (APIENTRY * m_pEnableVertexAttribArrayARB) (GLuint);
		GLvoid            (APIENTRY * m_pGenProgramsARB) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetProgramEnvParameterdvARB) (GLenum, GLuint, GLdouble *);
		GLvoid            (APIENTRY * m_pGetProgramEnvParameterfvARB) (GLenum, GLuint, GLfloat *);
		GLvoid            (APIENTRY * m_pGetProgramivARB) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetProgramLocalParameterdvARB) (GLenum, GLuint, GLdouble *);
		GLvoid            (APIENTRY * m_pGetProgramLocalParameterfvARB) (GLenum, GLuint, GLfloat *);
		GLvoid            (APIENTRY * m_pGetProgramStringARB) (GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetVertexAttribdvARB) (GLuint, GLenum, GLdouble *);
		GLvoid            (APIENTRY * m_pGetVertexAttribfvARB) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetVertexAttribivARB) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetVertexAttribPointervARB) (GLuint, GLenum, GLvoid * *);
		GLboolean         (APIENTRY * m_pIsProgramARB) (GLuint);
		GLvoid            (APIENTRY * m_pProgramEnvParameter4dARB) (GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pProgramEnvParameter4dvARB) (GLenum, GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pProgramEnvParameter4fARB) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pProgramEnvParameter4fvARB) (GLenum, GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pProgramLocalParameter4dARB) (GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pProgramLocalParameter4dvARB) (GLenum, GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pProgramLocalParameter4fARB) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pProgramLocalParameter4fvARB) (GLenum, GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pProgramStringARB) (GLenum, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1dARB) (GLuint, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib1dvARB) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1fARB) (GLuint, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib1fvARB) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1sARB) (GLuint, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib1svARB) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2dARB) (GLuint, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib2dvARB) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2fARB) (GLuint, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib2fvARB) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2sARB) (GLuint, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib2svARB) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3dARB) (GLuint, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib3dvARB) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3fARB) (GLuint, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib3fvARB) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3sARB) (GLuint, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib3svARB) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4bvARB) (GLuint, GLbyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4dARB) (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib4dvARB) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4fARB) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib4fvARB) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4ivARB) (GLuint, GLint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4NbvARB) (GLuint, GLbyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4NivARB) (GLuint, GLint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4NsvARB) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4NubARB) (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
		GLvoid            (APIENTRY * m_pVertexAttrib4NubvARB) (GLuint, GLubyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4NuivARB) (GLuint, GLuint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4NusvARB) (GLuint, GLushort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4sARB) (GLuint, GLshort, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib4svARB) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4ubvARB) (GLuint, GLubyte const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4uivARB) (GLuint, GLuint const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4usvARB) (GLuint, GLushort const *);
		GLvoid            (APIENTRY * m_pVertexAttribPointerARB) (GLuint, GLint, GLenum, GLboolean, GLsizei, GLvoid const *);

	#endif // GL_ARB_vertex_program_OGLEXT

	// - -[ gl_arb_vertex_shader ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_vertex_shader_OGLEXT

		GLvoid            (APIENTRY * m_pBindAttribLocationARB) (GLhandleARB, GLuint, GLcharARB const *);
		GLvoid            (APIENTRY * m_pGetActiveAttribARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
		GLint             (APIENTRY * m_pGetAttribLocationARB) (GLhandleARB, GLcharARB const *);

	#endif // GL_ARB_vertex_shader_OGLEXT

	// - -[ gl_arb_window_pos ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ARB_window_pos_OGLEXT

		GLvoid            (APIENTRY * m_pWindowPos2dARB) (GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pWindowPos2dvARB) (GLdouble const *);
		GLvoid            (APIENTRY * m_pWindowPos2fARB) (GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pWindowPos2fvARB) (GLfloat const *);
		GLvoid            (APIENTRY * m_pWindowPos2iARB) (GLint, GLint);
		GLvoid            (APIENTRY * m_pWindowPos2ivARB) (GLint const *);
		GLvoid            (APIENTRY * m_pWindowPos2sARB) (GLshort, GLshort);
		GLvoid            (APIENTRY * m_pWindowPos2svARB) (GLshort const *);
		GLvoid            (APIENTRY * m_pWindowPos3dARB) (GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pWindowPos3dvARB) (GLdouble const *);
		GLvoid            (APIENTRY * m_pWindowPos3fARB) (GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pWindowPos3fvARB) (GLfloat const *);
		GLvoid            (APIENTRY * m_pWindowPos3iARB) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pWindowPos3ivARB) (GLint const *);
		GLvoid            (APIENTRY * m_pWindowPos3sARB) (GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pWindowPos3svARB) (GLshort const *);

	#endif // GL_ARB_window_pos_OGLEXT

	// - -[ gl_ati_draw_buffers ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_draw_buffers_OGLEXT

		GLvoid            (APIENTRY * m_pDrawBuffersATI) (GLsizei, GLenum const *);

	#endif // GL_ATI_draw_buffers_OGLEXT

	// - -[ gl_ati_element_array ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_element_array_OGLEXT

		GLvoid            (APIENTRY * m_pDrawElementArrayATI) (GLenum, GLsizei);
		GLvoid            (APIENTRY * m_pDrawRangeElementArrayATI) (GLenum, GLuint, GLuint, GLsizei);
		GLvoid            (APIENTRY * m_pElementPointerATI) (GLenum, GLvoid const *);

	#endif // GL_ATI_element_array_OGLEXT

	// - -[ gl_ati_envmap_bumpmap ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_envmap_bumpmap_OGLEXT

		GLvoid            (APIENTRY * m_pGetTexBumpParameterfvATI) (GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetTexBumpParameterivATI) (GLenum, GLint *);
		GLvoid            (APIENTRY * m_pTexBumpParameterfvATI) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexBumpParameterivATI) (GLenum, GLint const *);

	#endif // GL_ATI_envmap_bumpmap_OGLEXT

	// - -[ gl_ati_fragment_shader ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_fragment_shader_OGLEXT

		GLvoid            (APIENTRY * m_pAlphaFragmentOp1ATI) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pAlphaFragmentOp2ATI) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pAlphaFragmentOp3ATI) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pBeginFragmentShaderATI) ();
		GLvoid            (APIENTRY * m_pBindFragmentShaderATI) (GLuint);
		GLvoid            (APIENTRY * m_pColorFragmentOp1ATI) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pColorFragmentOp2ATI) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pColorFragmentOp3ATI) (GLenum, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pDeleteFragmentShaderATI) (GLuint);
		GLvoid            (APIENTRY * m_pEndFragmentShaderATI) ();
		GLuint            (APIENTRY * m_pGenFragmentShadersATI) (GLuint);
		GLvoid            (APIENTRY * m_pPassTexCoordATI) (GLuint, GLuint, GLenum);
		GLvoid            (APIENTRY * m_pSampleMapATI) (GLuint, GLuint, GLenum);
		GLvoid            (APIENTRY * m_pSetFragmentShaderConstantATI) (GLuint, GLfloat const *);

	#endif // GL_ATI_fragment_shader_OGLEXT

	// - -[ gl_ati_map_object_buffer ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_map_object_buffer_OGLEXT

		GLvoid *          (APIENTRY * m_pMapObjectBufferATI) (GLuint);
		GLvoid            (APIENTRY * m_pUnmapObjectBufferATI) (GLuint);

	#endif // GL_ATI_map_object_buffer_OGLEXT

	// - -[ gl_ati_pn_triangles ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_pn_triangles_OGLEXT

		GLvoid            (APIENTRY * m_pPNTrianglesfATI) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pPNTrianglesiATI) (GLenum, GLint);

	#endif // GL_ATI_pn_triangles_OGLEXT

	// - -[ gl_ati_separate_stencil ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_separate_stencil_OGLEXT

		GLvoid            (APIENTRY * m_pStencilFuncSeparateATI) (GLenum, GLenum, GLint, GLuint);
		GLvoid            (APIENTRY * m_pStencilOpSeparateATI) (GLenum, GLenum, GLenum, GLenum);

	#endif // GL_ATI_separate_stencil_OGLEXT

	// - -[ gl_ati_vertex_array_object ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_vertex_array_object_OGLEXT

		GLvoid            (APIENTRY * m_pArrayObjectATI) (GLenum, GLint, GLenum, GLsizei, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pFreeObjectBufferATI) (GLuint);
		GLvoid            (APIENTRY * m_pGetArrayObjectfvATI) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetArrayObjectivATI) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetObjectBufferfvATI) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetObjectBufferivATI) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetVariantArrayObjectfvATI) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetVariantArrayObjectivATI) (GLuint, GLenum, GLint *);
		GLboolean         (APIENTRY * m_pIsObjectBufferATI) (GLuint);
		GLuint            (APIENTRY * m_pNewObjectBufferATI) (GLsizei, GLvoid const *, GLenum);
		GLvoid            (APIENTRY * m_pUpdateObjectBufferATI) (GLuint, GLuint, GLsizei, GLvoid const *, GLenum);
		GLvoid            (APIENTRY * m_pVariantArrayObjectATI) (GLuint, GLenum, GLsizei, GLuint, GLuint);

	#endif // GL_ATI_vertex_array_object_OGLEXT

	// - -[ gl_ati_vertex_attrib_array_object ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_vertex_attrib_array_object_OGLEXT

		GLvoid            (APIENTRY * m_pGetVertexAttribArrayObjectfvATI) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetVertexAttribArrayObjectivATI) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pVertexAttribArrayObjectATI) (GLuint, GLint, GLenum, GLboolean, GLsizei, GLuint, GLuint);

	#endif // GL_ATI_vertex_attrib_array_object_OGLEXT

	// - -[ gl_ati_vertex_streams ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_ATI_vertex_streams_OGLEXT

		GLvoid            (APIENTRY * m_pClientActiveVertexStreamATI) (GLenum);
		GLvoid            (APIENTRY * m_pNormalStream3bATI) (GLenum, GLbyte, GLbyte, GLbyte);
		GLvoid            (APIENTRY * m_pNormalStream3bvATI) (GLenum, GLbyte const *);
		GLvoid            (APIENTRY * m_pNormalStream3dATI) (GLenum, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pNormalStream3dvATI) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pNormalStream3fATI) (GLenum, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pNormalStream3fvATI) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pNormalStream3iATI) (GLenum, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pNormalStream3ivATI) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pNormalStream3sATI) (GLenum, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pNormalStream3svATI) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexBlendEnvfATI) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pVertexBlendEnviATI) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pVertexStream1dATI) (GLenum, GLdouble);
		GLvoid            (APIENTRY * m_pVertexStream1dvATI) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexStream1fATI) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pVertexStream1fvATI) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexStream1iATI) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pVertexStream1ivATI) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pVertexStream1sATI) (GLenum, GLshort);
		GLvoid            (APIENTRY * m_pVertexStream1svATI) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexStream2dATI) (GLenum, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexStream2dvATI) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexStream2fATI) (GLenum, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexStream2fvATI) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexStream2iATI) (GLenum, GLint, GLint);
		GLvoid            (APIENTRY * m_pVertexStream2ivATI) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pVertexStream2sATI) (GLenum, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexStream2svATI) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexStream3dATI) (GLenum, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexStream3dvATI) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexStream3fATI) (GLenum, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexStream3fvATI) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexStream3iATI) (GLenum, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pVertexStream3ivATI) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pVertexStream3sATI) (GLenum, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexStream3svATI) (GLenum, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexStream4dATI) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexStream4dvATI) (GLenum, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexStream4fATI) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexStream4fvATI) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexStream4iATI) (GLenum, GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pVertexStream4ivATI) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pVertexStream4sATI) (GLenum, GLshort, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexStream4svATI) (GLenum, GLshort const *);

	#endif // GL_ATI_vertex_streams_OGLEXT

	// - -[ gl_ext_blend_color ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_blend_color_OGLEXT

		GLvoid            (APIENTRY * m_pBlendColorEXT) (GLclampf, GLclampf, GLclampf, GLclampf);

	#endif // GL_EXT_blend_color_OGLEXT

	// - -[ gl_ext_blend_equation_separate ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_blend_equation_separate_OGLEXT

		GLvoid            (APIENTRY * m_pBlendEquationSeparateEXT) (GLenum, GLenum);

	#endif // GL_EXT_blend_equation_separate_OGLEXT

	// - -[ gl_ext_blend_func_separate ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_blend_func_separate_OGLEXT

		GLvoid            (APIENTRY * m_pBlendFuncSeparateEXT) (GLenum, GLenum, GLenum, GLenum);

	#endif // GL_EXT_blend_func_separate_OGLEXT

	// - -[ gl_ext_blend_minmax ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_blend_minmax_OGLEXT

		GLvoid            (APIENTRY * m_pBlendEquationEXT) (GLenum);

	#endif // GL_EXT_blend_minmax_OGLEXT

	// - -[ gl_ext_color_subtable ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_color_subtable_OGLEXT

//CT
	#if defined(__APPLE__) && defined(__MACH__)
		GLvoid            (APIENTRY * m_pColorSubTableEXT) (GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
	#endif // MACOSX

		GLvoid            (APIENTRY * m_pCopyColorSubTableEXT) (GLenum, GLsizei, GLint, GLint, GLsizei);

	#endif // GL_EXT_color_subtable_OGLEXT

	// - -[ gl_ext_compiled_vertex_array ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_compiled_vertex_array_OGLEXT

		GLvoid            (APIENTRY * m_pLockArraysEXT) (GLint, GLsizei);
		GLvoid            (APIENTRY * m_pUnlockArraysEXT) ();

	#endif // GL_EXT_compiled_vertex_array_OGLEXT

	// - -[ gl_ext_convolution ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_convolution_OGLEXT

		GLvoid            (APIENTRY * m_pConvolutionFilter1DEXT) (GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pConvolutionFilter2DEXT) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pConvolutionParameterfEXT) (GLenum, GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pConvolutionParameterfvEXT) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pConvolutionParameteriEXT) (GLenum, GLenum, GLint);
		GLvoid            (APIENTRY * m_pConvolutionParameterivEXT) (GLenum, GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pCopyConvolutionFilter1DEXT) (GLenum, GLenum, GLint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pCopyConvolutionFilter2DEXT) (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
		GLvoid            (APIENTRY * m_pGetConvolutionFilterEXT) (GLenum, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetConvolutionParameterfvEXT) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetConvolutionParameterivEXT) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetSeparableFilterEXT) (GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
		GLvoid            (APIENTRY * m_pSeparableFilter2DEXT) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *, GLvoid const *);

	#endif // GL_EXT_convolution_OGLEXT

	// - -[ gl_ext_coordinate_frame ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_coordinate_frame_OGLEXT

		GLvoid            (APIENTRY * m_pBinormal3bEXT) (GLbyte, GLbyte, GLbyte);
		GLvoid            (APIENTRY * m_pBinormal3bvEXT) (GLbyte const *);
		GLvoid            (APIENTRY * m_pBinormal3dEXT) (GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pBinormal3dvEXT) (GLdouble const *);
		GLvoid            (APIENTRY * m_pBinormal3fEXT) (GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pBinormal3fvEXT) (GLfloat const *);
		GLvoid            (APIENTRY * m_pBinormal3iEXT) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pBinormal3ivEXT) (GLint const *);
		GLvoid            (APIENTRY * m_pBinormal3sEXT) (GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pBinormal3svEXT) (GLshort const *);
		GLvoid            (APIENTRY * m_pBinormalPointerEXT) (GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pTangent3bEXT) (GLbyte, GLbyte, GLbyte);
		GLvoid            (APIENTRY * m_pTangent3bvEXT) (GLbyte const *);
		GLvoid            (APIENTRY * m_pTangent3dEXT) (GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pTangent3dvEXT) (GLdouble const *);
		GLvoid            (APIENTRY * m_pTangent3fEXT) (GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTangent3fvEXT) (GLfloat const *);
		GLvoid            (APIENTRY * m_pTangent3iEXT) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pTangent3ivEXT) (GLint const *);
		GLvoid            (APIENTRY * m_pTangent3sEXT) (GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pTangent3svEXT) (GLshort const *);
		GLvoid            (APIENTRY * m_pTangentPointerEXT) (GLenum, GLsizei, GLvoid const *);

	#endif // GL_EXT_coordinate_frame_OGLEXT

	// - -[ gl_ext_copy_texture ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_copy_texture_OGLEXT

		GLvoid            (APIENTRY * m_pCopyTexImage1DEXT) (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
		GLvoid            (APIENTRY * m_pCopyTexImage2DEXT) (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
		GLvoid            (APIENTRY * m_pCopyTexSubImage1DEXT) (GLenum, GLint, GLint, GLint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pCopyTexSubImage2DEXT) (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
		GLvoid            (APIENTRY * m_pCopyTexSubImage3DEXT) (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);

	#endif // GL_EXT_copy_texture_OGLEXT

	// - -[ gl_ext_cull_vertex ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_cull_vertex_OGLEXT

		GLvoid            (APIENTRY * m_pCullParameterdvEXT) (GLenum, GLdouble *);
		GLvoid            (APIENTRY * m_pCullParameterfvEXT) (GLenum, GLfloat *);

	#endif // GL_EXT_cull_vertex_OGLEXT

	// - -[ gl_ext_depth_bounds_test ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_depth_bounds_test_OGLEXT

		GLvoid            (APIENTRY * m_pDepthBoundsEXT) (GLclampd, GLclampd);

	#endif // GL_EXT_depth_bounds_test_OGLEXT

	// - -[ gl_ext_draw_range_elements ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_draw_range_elements_OGLEXT

		GLvoid            (APIENTRY * m_pDrawRangeElementsEXT) (GLenum, GLuint, GLuint, GLsizei, GLenum, GLvoid const *);

	#endif // GL_EXT_draw_range_elements_OGLEXT

	// - -[ gl_ext_fog_coord ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_fog_coord_OGLEXT

		GLvoid            (APIENTRY * m_pFogCoorddEXT) (GLdouble);
		GLvoid            (APIENTRY * m_pFogCoorddvEXT) (GLdouble const *);
		GLvoid            (APIENTRY * m_pFogCoordfEXT) (GLfloat);
		GLvoid            (APIENTRY * m_pFogCoordfvEXT) (GLfloat const *);
		GLvoid            (APIENTRY * m_pFogCoordPointerEXT) (GLenum, GLsizei, GLvoid const *);

	#endif // GL_EXT_fog_coord_OGLEXT

	// - -[ gl_ext_framebuffer_object ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_framebuffer_object_OGLEXT

		GLvoid            (APIENTRY * m_pBindFramebufferEXT) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pBindRenderbufferEXT) (GLenum, GLuint);
		GLenum            (APIENTRY * m_pCheckFramebufferStatusEXT) (GLenum);
		GLvoid            (APIENTRY * m_pDeleteFramebuffersEXT) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pDeleteRenderbuffersEXT) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pFramebufferRenderbufferEXT) (GLenum, GLenum, GLenum, GLuint);
		GLvoid            (APIENTRY * m_pFramebufferTexture1DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
		GLvoid            (APIENTRY * m_pFramebufferTexture2DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
		GLvoid            (APIENTRY * m_pFramebufferTexture3DEXT) (GLenum, GLenum, GLenum, GLuint, GLint, GLint);
		GLvoid            (APIENTRY * m_pGenerateMipmapEXT) (GLenum);
		GLvoid            (APIENTRY * m_pGenFramebuffersEXT) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGenRenderbuffersEXT) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetFramebufferAttachmentParameterivEXT) (GLenum, GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetRenderbufferParameterivEXT) (GLenum, GLenum, GLint *);
		GLboolean         (APIENTRY * m_pIsFramebufferEXT) (GLuint);
		GLboolean         (APIENTRY * m_pIsRenderbufferEXT) (GLuint);
		GLvoid            (APIENTRY * m_pRenderbufferStorageEXT) (GLenum, GLenum, GLsizei, GLsizei);

	#endif // GL_EXT_framebuffer_object_OGLEXT

	// - -[ gl_ext_histogram ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_histogram_OGLEXT

		GLvoid            (APIENTRY * m_pGetHistogramEXT) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetHistogramParameterfvEXT) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetHistogramParameterivEXT) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetMinmaxEXT) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetMinmaxParameterfvEXT) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetMinmaxParameterivEXT) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pHistogramEXT) (GLenum, GLsizei, GLenum, GLboolean);
		GLvoid            (APIENTRY * m_pMinmaxEXT) (GLenum, GLenum, GLboolean);
		GLvoid            (APIENTRY * m_pResetHistogramEXT) (GLenum);
		GLvoid            (APIENTRY * m_pResetMinmaxEXT) (GLenum);

	#endif // GL_EXT_histogram_OGLEXT

	// - -[ gl_ext_index_func ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_index_func_OGLEXT

		GLvoid            (APIENTRY * m_pIndexFuncEXT) (GLenum, GLclampf);

	#endif // GL_EXT_index_func_OGLEXT

	// - -[ gl_ext_index_material ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_index_material_OGLEXT

		GLvoid            (APIENTRY * m_pIndexMaterialEXT) (GLenum, GLenum);

	#endif // GL_EXT_index_material_OGLEXT

	// - -[ gl_ext_light_texture ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_light_texture_OGLEXT

		GLvoid            (APIENTRY * m_pApplyTextureEXT) (GLenum);
		GLvoid            (APIENTRY * m_pTextureLightEXT) (GLenum);
		GLvoid            (APIENTRY * m_pTextureMaterialEXT) (GLenum, GLenum);

	#endif // GL_EXT_light_texture_OGLEXT

	// - -[ gl_ext_multi_draw_arrays ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_multi_draw_arrays_OGLEXT

		GLvoid            (APIENTRY * m_pMultiDrawArraysEXT) (GLenum, GLint *, GLsizei *, GLsizei);
		GLvoid            (APIENTRY * m_pMultiDrawElementsEXT) (GLenum, GLsizei const *, GLenum, GLvoid const * *, GLsizei);

	#endif // GL_EXT_multi_draw_arrays_OGLEXT

	// - -[ gl_ext_multisample ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_multisample_OGLEXT

		GLvoid            (APIENTRY * m_pSampleMaskEXT) (GLclampf, GLboolean);
		GLvoid            (APIENTRY * m_pSamplePatternEXT) (GLenum);

	#endif // GL_EXT_multisample_OGLEXT

	// - -[ gl_ext_paletted_texture ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_paletted_texture_OGLEXT

		GLvoid            (APIENTRY * m_pColorSubTableEXT) (GLenum, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pColorTableEXT) (GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pGetColorTableEXT) (GLenum, GLenum, GLenum, GLvoid *);
		GLvoid            (APIENTRY * m_pGetColorTableParameterfvEXT) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetColorTableParameterivEXT) (GLenum, GLenum, GLint *);

	#endif // GL_EXT_paletted_texture_OGLEXT

	// - -[ gl_ext_pixel_transform ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_pixel_transform_OGLEXT

		GLvoid            (APIENTRY * m_pPixelTransformParameterfEXT) (GLenum, GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pPixelTransformParameterfvEXT) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pPixelTransformParameteriEXT) (GLenum, GLenum, GLint);
		GLvoid            (APIENTRY * m_pPixelTransformParameterivEXT) (GLenum, GLenum, GLint const *);

	#endif // GL_EXT_pixel_transform_OGLEXT

	// - -[ gl_ext_point_parameters ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_point_parameters_OGLEXT

		GLvoid            (APIENTRY * m_pPointParameterfEXT) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pPointParameterfvEXT) (GLenum, GLfloat const *);

	#endif // GL_EXT_point_parameters_OGLEXT

	// - -[ gl_ext_polygon_offset ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_polygon_offset_OGLEXT

		GLvoid            (APIENTRY * m_pPolygonOffsetEXT) (GLfloat, GLfloat);

	#endif // GL_EXT_polygon_offset_OGLEXT

	// - -[ gl_ext_secondary_color ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_secondary_color_OGLEXT

		GLvoid            (APIENTRY * m_pSecondaryColor3bEXT) (GLbyte, GLbyte, GLbyte);
		GLvoid            (APIENTRY * m_pSecondaryColor3bvEXT) (GLbyte const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3dEXT) (GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pSecondaryColor3dvEXT) (GLdouble const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3fEXT) (GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pSecondaryColor3fvEXT) (GLfloat const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3iEXT) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pSecondaryColor3ivEXT) (GLint const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3sEXT) (GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pSecondaryColor3svEXT) (GLshort const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3ubEXT) (GLubyte, GLubyte, GLubyte);
		GLvoid            (APIENTRY * m_pSecondaryColor3ubvEXT) (GLubyte const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3uiEXT) (GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pSecondaryColor3uivEXT) (GLuint const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3usEXT) (GLushort, GLushort, GLushort);
		GLvoid            (APIENTRY * m_pSecondaryColor3usvEXT) (GLushort const *);
		GLvoid            (APIENTRY * m_pSecondaryColorPointerEXT) (GLint, GLenum, GLsizei, GLvoid const *);

	#endif // GL_EXT_secondary_color_OGLEXT

	// - -[ gl_ext_stencil_two_side ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_stencil_two_side_OGLEXT

		GLvoid            (APIENTRY * m_pActiveStencilFaceEXT) (GLenum);

	#endif // GL_EXT_stencil_two_side_OGLEXT

	// - -[ gl_ext_subtexture ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_subtexture_OGLEXT

		GLvoid            (APIENTRY * m_pTexSubImage1DEXT) (GLenum, GLint, GLint, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pTexSubImage2DEXT) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

	#endif // GL_EXT_subtexture_OGLEXT

	// - -[ gl_ext_texture3d ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_texture3D_OGLEXT

		GLvoid            (APIENTRY * m_pTexImage3DEXT) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pTexSubImage3DEXT) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

	#endif // GL_EXT_texture3D_OGLEXT

	// - -[ gl_ext_texture_object ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_texture_object_OGLEXT

		GLboolean         (APIENTRY * m_pAreTexturesResidentEXT) (GLsizei, GLuint const *, GLboolean *);
		GLvoid            (APIENTRY * m_pBindTextureEXT) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pDeleteTexturesEXT) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pGenTexturesEXT) (GLsizei, GLuint *);
		GLboolean         (APIENTRY * m_pIsTextureEXT) (GLuint);
		GLvoid            (APIENTRY * m_pPrioritizeTexturesEXT) (GLsizei, GLuint const *, GLclampf const *);

	#endif // GL_EXT_texture_object_OGLEXT

	// - -[ gl_ext_texture_perturb_normal ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_texture_perturb_normal_OGLEXT

		GLvoid            (APIENTRY * m_pTextureNormalEXT) (GLenum);

	#endif // GL_EXT_texture_perturb_normal_OGLEXT

	// - -[ gl_ext_vertex_array ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_vertex_array_OGLEXT

		GLvoid            (APIENTRY * m_pArrayElementEXT) (GLint);
		GLvoid            (APIENTRY * m_pColorPointerEXT) (GLint, GLenum, GLsizei, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pDrawArraysEXT) (GLenum, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pEdgeFlagPointerEXT) (GLsizei, GLsizei, GLboolean const *);
		GLvoid            (APIENTRY * m_pGetPointervEXT) (GLenum, GLvoid * *);
		GLvoid            (APIENTRY * m_pIndexPointerEXT) (GLenum, GLsizei, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pNormalPointerEXT) (GLenum, GLsizei, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pTexCoordPointerEXT) (GLint, GLenum, GLsizei, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pVertexPointerEXT) (GLint, GLenum, GLsizei, GLsizei, GLvoid const *);

	#endif // GL_EXT_vertex_array_OGLEXT

	// - -[ gl_ext_vertex_shader ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_vertex_shader_OGLEXT

		GLvoid            (APIENTRY * m_pBeginVertexShaderEXT) ();
		GLuint            (APIENTRY * m_pBindLightParameterEXT) (GLenum, GLenum);
		GLuint            (APIENTRY * m_pBindMaterialParameterEXT) (GLenum, GLenum);
		GLuint            (APIENTRY * m_pBindParameterEXT) (GLenum);
		GLuint            (APIENTRY * m_pBindTexGenParameterEXT) (GLenum, GLenum, GLenum);
		GLuint            (APIENTRY * m_pBindTextureUnitParameterEXT) (GLenum, GLenum);
		GLvoid            (APIENTRY * m_pBindVertexShaderEXT) (GLuint);
		GLvoid            (APIENTRY * m_pDeleteVertexShaderEXT) (GLuint);
		GLvoid            (APIENTRY * m_pDisableVariantClientStateEXT) (GLuint);
		GLvoid            (APIENTRY * m_pEnableVariantClientStateEXT) (GLuint);
		GLvoid            (APIENTRY * m_pEndVertexShaderEXT) ();
		GLvoid            (APIENTRY * m_pExtractComponentEXT) (GLuint, GLuint, GLuint);
		GLuint            (APIENTRY * m_pGenSymbolsEXT) (GLenum, GLenum, GLenum, GLuint);
		GLuint            (APIENTRY * m_pGenVertexShadersEXT) (GLuint);
		GLvoid            (APIENTRY * m_pGetInvariantBooleanvEXT) (GLuint, GLenum, GLboolean *);
		GLvoid            (APIENTRY * m_pGetInvariantFloatvEXT) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetInvariantIntegervEXT) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetLocalConstantBooleanvEXT) (GLuint, GLenum, GLboolean *);
		GLvoid            (APIENTRY * m_pGetLocalConstantFloatvEXT) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetLocalConstantIntegervEXT) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetVariantBooleanvEXT) (GLuint, GLenum, GLboolean *);
		GLvoid            (APIENTRY * m_pGetVariantFloatvEXT) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetVariantIntegervEXT) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetVariantPointervEXT) (GLuint, GLenum, GLvoid * *);
		GLvoid            (APIENTRY * m_pInsertComponentEXT) (GLuint, GLuint, GLuint);
		GLboolean         (APIENTRY * m_pIsVariantEnabledEXT) (GLuint, GLenum);
		GLvoid            (APIENTRY * m_pSetInvariantEXT) (GLuint, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pSetLocalConstantEXT) (GLuint, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pShaderOp1EXT) (GLenum, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pShaderOp2EXT) (GLenum, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pShaderOp3EXT) (GLenum, GLuint, GLuint, GLuint, GLuint);
		GLvoid            (APIENTRY * m_pSwizzleEXT) (GLuint, GLuint, GLenum, GLenum, GLenum, GLenum);
		GLvoid            (APIENTRY * m_pVariantbvEXT) (GLuint, GLbyte const *);
		GLvoid            (APIENTRY * m_pVariantdvEXT) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVariantfvEXT) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVariantivEXT) (GLuint, GLint const *);
		GLvoid            (APIENTRY * m_pVariantPointerEXT) (GLuint, GLenum, GLuint, GLvoid const *);
		GLvoid            (APIENTRY * m_pVariantsvEXT) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVariantubvEXT) (GLuint, GLubyte const *);
		GLvoid            (APIENTRY * m_pVariantuivEXT) (GLuint, GLuint const *);
		GLvoid            (APIENTRY * m_pVariantusvEXT) (GLuint, GLushort const *);
		GLvoid            (APIENTRY * m_pWriteMaskEXT) (GLuint, GLuint, GLenum, GLenum, GLenum, GLenum);

	#endif // GL_EXT_vertex_shader_OGLEXT

	// - -[ gl_ext_vertex_weighting ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_EXT_vertex_weighting_OGLEXT

		GLvoid            (APIENTRY * m_pVertexWeightfEXT) (GLfloat);
		GLvoid            (APIENTRY * m_pVertexWeightfvEXT) (GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexWeightPointerEXT) (GLsizei, GLenum, GLsizei, GLvoid const *);

	#endif // GL_EXT_vertex_weighting_OGLEXT

	// - -[ gl_gremedy_string_marker ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_GREMEDY_string_marker_OGLEXT

		GLvoid            (APIENTRY * m_pStringMarkerGREMEDY) (GLsizei, GLvoid const *);

	#endif // GL_GREMEDY_string_marker_OGLEXT

	// - -[ gl_hp_image_transform ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_HP_image_transform_OGLEXT

		GLvoid            (APIENTRY * m_pGetImageTransformParameterfvHP) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetImageTransformParameterivHP) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pImageTransformParameterfHP) (GLenum, GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pImageTransformParameterfvHP) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pImageTransformParameteriHP) (GLenum, GLenum, GLint);
		GLvoid            (APIENTRY * m_pImageTransformParameterivHP) (GLenum, GLenum, GLint const *);

	#endif // GL_HP_image_transform_OGLEXT

	// - -[ gl_ibm_multimode_draw_arrays ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_IBM_multimode_draw_arrays_OGLEXT

		GLvoid            (APIENTRY * m_pMultiModeDrawArraysIBM) (GLenum const *, GLint const *, GLsizei const *, GLsizei, GLint);
		GLvoid            (APIENTRY * m_pMultiModeDrawElementsIBM) (GLenum const *, GLsizei const *, GLenum, GLvoid const * const *, GLsizei, GLint);

	#endif // GL_IBM_multimode_draw_arrays_OGLEXT

	// - -[ gl_ibm_vertex_array_lists ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_IBM_vertex_array_lists_OGLEXT

		GLvoid            (APIENTRY * m_pColorPointerListIBM) (GLint, GLenum, GLint, GLvoid const * *, GLint);
		GLvoid            (APIENTRY * m_pEdgeFlagPointerListIBM) (GLint, GLboolean const * *, GLint);
		GLvoid            (APIENTRY * m_pFogCoordPointerListIBM) (GLenum, GLint, GLvoid const * *, GLint);
		GLvoid            (APIENTRY * m_pIndexPointerListIBM) (GLenum, GLint, GLvoid const * *, GLint);
		GLvoid            (APIENTRY * m_pNormalPointerListIBM) (GLenum, GLint, GLvoid const * *, GLint);
		GLvoid            (APIENTRY * m_pSecondaryColorPointerListIBM) (GLint, GLenum, GLint, GLvoid const * *, GLint);
		GLvoid            (APIENTRY * m_pTexCoordPointerListIBM) (GLint, GLenum, GLint, GLvoid const * *, GLint);
		GLvoid            (APIENTRY * m_pVertexPointerListIBM) (GLint, GLenum, GLint, GLvoid const * *, GLint);

	#endif // GL_IBM_vertex_array_lists_OGLEXT

	// - -[ gl_ingr_blend_func_separate ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_INGR_blend_func_separate_OGLEXT

		GLvoid            (APIENTRY * m_pBlendFuncSeparateINGR) (GLenum, GLenum, GLenum, GLenum);

	#endif // GL_INGR_blend_func_separate_OGLEXT

	// - -[ gl_intel_parallel_arrays ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_INTEL_parallel_arrays_OGLEXT

		GLvoid            (APIENTRY * m_pColorPointervINTEL) (GLint, GLenum, GLvoid const * *);
		GLvoid            (APIENTRY * m_pNormalPointervINTEL) (GLenum, GLvoid const * *);
		GLvoid            (APIENTRY * m_pTexCoordPointervINTEL) (GLint, GLenum, GLvoid const * *);
		GLvoid            (APIENTRY * m_pVertexPointervINTEL) (GLint, GLenum, GLvoid const * *);

	#endif // GL_INTEL_parallel_arrays_OGLEXT

	// - -[ gl_mesa_resize_buffers ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_MESA_resize_buffers_OGLEXT

		GLvoid            (APIENTRY * m_pResizeBuffersMESA) ();

	#endif // GL_MESA_resize_buffers_OGLEXT

	// - -[ gl_mesa_window_pos ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_MESA_window_pos_OGLEXT

		GLvoid            (APIENTRY * m_pWindowPos2dMESA) (GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pWindowPos2dvMESA) (GLdouble const *);
		GLvoid            (APIENTRY * m_pWindowPos2fMESA) (GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pWindowPos2fvMESA) (GLfloat const *);
		GLvoid            (APIENTRY * m_pWindowPos2iMESA) (GLint, GLint);
		GLvoid            (APIENTRY * m_pWindowPos2ivMESA) (GLint const *);
		GLvoid            (APIENTRY * m_pWindowPos2sMESA) (GLshort, GLshort);
		GLvoid            (APIENTRY * m_pWindowPos2svMESA) (GLshort const *);
		GLvoid            (APIENTRY * m_pWindowPos3dMESA) (GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pWindowPos3dvMESA) (GLdouble const *);
		GLvoid            (APIENTRY * m_pWindowPos3fMESA) (GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pWindowPos3fvMESA) (GLfloat const *);
		GLvoid            (APIENTRY * m_pWindowPos3iMESA) (GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pWindowPos3ivMESA) (GLint const *);
		GLvoid            (APIENTRY * m_pWindowPos3sMESA) (GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pWindowPos3svMESA) (GLshort const *);
		GLvoid            (APIENTRY * m_pWindowPos4dMESA) (GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pWindowPos4dvMESA) (GLdouble const *);
		GLvoid            (APIENTRY * m_pWindowPos4fMESA) (GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pWindowPos4fvMESA) (GLfloat const *);
		GLvoid            (APIENTRY * m_pWindowPos4iMESA) (GLint, GLint, GLint, GLint);
		GLvoid            (APIENTRY * m_pWindowPos4ivMESA) (GLint const *);
		GLvoid            (APIENTRY * m_pWindowPos4sMESA) (GLshort, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pWindowPos4svMESA) (GLshort const *);

	#endif // GL_MESA_window_pos_OGLEXT

	// - -[ gl_nv_element_array ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_element_array_OGLEXT

		GLvoid            (APIENTRY * m_pDrawElementArrayNV) (GLenum, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pDrawRangeElementArrayNV) (GLenum, GLuint, GLuint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pElementPointerNV) (GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pMultiDrawElementArrayNV) (GLenum, GLint const *, GLsizei const *, GLsizei);
		GLvoid            (APIENTRY * m_pMultiDrawRangeElementArrayNV) (GLenum, GLuint, GLuint, GLint const *, GLsizei const *, GLsizei);

	#endif // GL_NV_element_array_OGLEXT

	// - -[ gl_nv_evaluators ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_evaluators_OGLEXT

		GLvoid            (APIENTRY * m_pEvalMapsNV) (GLenum, GLenum);
		GLvoid            (APIENTRY * m_pGetMapAttribParameterfvNV) (GLenum, GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetMapAttribParameterivNV) (GLenum, GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetMapControlPointsNV) (GLenum, GLuint, GLenum, GLsizei, GLsizei, GLboolean, GLvoid *);
		GLvoid            (APIENTRY * m_pGetMapParameterfvNV) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetMapParameterivNV) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pMapControlPointsNV) (GLenum, GLuint, GLenum, GLsizei, GLsizei, GLint, GLint, GLboolean, GLvoid const *);
		GLvoid            (APIENTRY * m_pMapParameterfvNV) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pMapParameterivNV) (GLenum, GLenum, GLint const *);

	#endif // GL_NV_evaluators_OGLEXT

	// - -[ gl_nv_fence ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_fence_OGLEXT

		GLvoid            (APIENTRY * m_pDeleteFencesNV) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pFinishFenceNV) (GLuint);
		GLvoid            (APIENTRY * m_pGenFencesNV) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetFenceivNV) (GLuint, GLenum, GLint *);
		GLboolean         (APIENTRY * m_pIsFenceNV) (GLuint);
		GLvoid            (APIENTRY * m_pSetFenceNV) (GLuint, GLenum);
		GLboolean         (APIENTRY * m_pTestFenceNV) (GLuint);

	#endif // GL_NV_fence_OGLEXT

	// - -[ gl_nv_fragment_program ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_fragment_program_OGLEXT

		GLvoid            (APIENTRY * m_pGetProgramNamedParameterdvNV) (GLuint, GLsizei, GLubyte const *, GLdouble *);
		GLvoid            (APIENTRY * m_pGetProgramNamedParameterfvNV) (GLuint, GLsizei, GLubyte const *, GLfloat *);
		GLvoid            (APIENTRY * m_pProgramNamedParameter4dNV) (GLuint, GLsizei, GLubyte const *, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pProgramNamedParameter4dvNV) (GLuint, GLsizei, GLubyte const *, GLdouble const *);
		GLvoid            (APIENTRY * m_pProgramNamedParameter4fNV) (GLuint, GLsizei, GLubyte const *, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pProgramNamedParameter4fvNV) (GLuint, GLsizei, GLubyte const *, GLfloat const *);

	#endif // GL_NV_fragment_program_OGLEXT

	// - -[ gl_nv_half_float ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_half_float_OGLEXT

		GLvoid            (APIENTRY * m_pColor3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pColor3hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pColor4hNV) (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pColor4hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pFogCoordhNV) (GLhalfNV);
		GLvoid            (APIENTRY * m_pFogCoordhvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord1hNV) (GLenum, GLhalfNV);
		GLvoid            (APIENTRY * m_pMultiTexCoord1hvNV) (GLenum, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord2hNV) (GLenum, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pMultiTexCoord2hvNV) (GLenum, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord3hNV) (GLenum, GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pMultiTexCoord3hvNV) (GLenum, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pMultiTexCoord4hNV) (GLenum, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pMultiTexCoord4hvNV) (GLenum, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pNormal3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pNormal3hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pSecondaryColor3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pSecondaryColor3hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pTexCoord1hNV) (GLhalfNV);
		GLvoid            (APIENTRY * m_pTexCoord1hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pTexCoord2hNV) (GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pTexCoord2hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pTexCoord3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pTexCoord3hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pTexCoord4hNV) (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pTexCoord4hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertex2hNV) (GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pVertex2hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertex3hNV) (GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pVertex3hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertex4hNV) (GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pVertex4hvNV) (GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1hNV) (GLuint, GLhalfNV);
		GLvoid            (APIENTRY * m_pVertexAttrib1hvNV) (GLuint, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2hNV) (GLuint, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pVertexAttrib2hvNV) (GLuint, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3hNV) (GLuint, GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pVertexAttrib3hvNV) (GLuint, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4hNV) (GLuint, GLhalfNV, GLhalfNV, GLhalfNV, GLhalfNV);
		GLvoid            (APIENTRY * m_pVertexAttrib4hvNV) (GLuint, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttribs1hvNV) (GLuint, GLsizei, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttribs2hvNV) (GLuint, GLsizei, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttribs3hvNV) (GLuint, GLsizei, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexAttribs4hvNV) (GLuint, GLsizei, GLhalfNV const *);
		GLvoid            (APIENTRY * m_pVertexWeighthNV) (GLhalfNV);
		GLvoid            (APIENTRY * m_pVertexWeighthvNV) (GLhalfNV const *);

	#endif // GL_NV_half_float_OGLEXT

	// - -[ gl_nv_occlusion_query ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_occlusion_query_OGLEXT

		GLvoid            (APIENTRY * m_pBeginOcclusionQueryNV) (GLuint);
		GLvoid            (APIENTRY * m_pDeleteOcclusionQueriesNV) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pEndOcclusionQueryNV) ();
		GLvoid            (APIENTRY * m_pGenOcclusionQueriesNV) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetOcclusionQueryivNV) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetOcclusionQueryuivNV) (GLuint, GLenum, GLuint *);
		GLboolean         (APIENTRY * m_pIsOcclusionQueryNV) (GLuint);

	#endif // GL_NV_occlusion_query_OGLEXT

	// - -[ gl_nv_pixel_data_range ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_pixel_data_range_OGLEXT

		GLvoid            (APIENTRY * m_pFlushPixelDataRangeNV) (GLenum);
		GLvoid            (APIENTRY * m_pPixelDataRangeNV) (GLenum, GLsizei, GLvoid *);

	#endif // GL_NV_pixel_data_range_OGLEXT

	// - -[ gl_nv_point_sprite ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_point_sprite_OGLEXT

		GLvoid            (APIENTRY * m_pPointParameteriNV) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pPointParameterivNV) (GLenum, GLint const *);

	#endif // GL_NV_point_sprite_OGLEXT

	// - -[ gl_nv_primitive_restart ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_primitive_restart_OGLEXT

		GLvoid            (APIENTRY * m_pPrimitiveRestartIndexNV) (GLuint);
		GLvoid            (APIENTRY * m_pPrimitiveRestartNV) ();

	#endif // GL_NV_primitive_restart_OGLEXT

	// - -[ gl_nv_register_combiners ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_register_combiners_OGLEXT

		GLvoid            (APIENTRY * m_pCombinerInputNV) (GLenum, GLenum, GLenum, GLenum, GLenum, GLenum);
		GLvoid            (APIENTRY * m_pCombinerOutputNV) (GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLboolean, GLboolean, GLboolean);
		GLvoid            (APIENTRY * m_pCombinerParameterfNV) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pCombinerParameterfvNV) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pCombinerParameteriNV) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pCombinerParameterivNV) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pFinalCombinerInputNV) (GLenum, GLenum, GLenum, GLenum);
		GLvoid            (APIENTRY * m_pGetCombinerInputParameterfvNV) (GLenum, GLenum, GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetCombinerInputParameterivNV) (GLenum, GLenum, GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetCombinerOutputParameterfvNV) (GLenum, GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetCombinerOutputParameterivNV) (GLenum, GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetFinalCombinerInputParameterfvNV) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetFinalCombinerInputParameterivNV) (GLenum, GLenum, GLint *);

	#endif // GL_NV_register_combiners_OGLEXT

	// - -[ gl_nv_register_combiners2 ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_register_combiners2_OGLEXT

		GLvoid            (APIENTRY * m_pCombinerStageParameterfvNV) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pGetCombinerStageParameterfvNV) (GLenum, GLenum, GLfloat *);

	#endif // GL_NV_register_combiners2_OGLEXT

	// - -[ gl_nv_stencil_two_side ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_stencil_two_side_OGLEXT

		GLvoid            (APIENTRY * m_pActiveStencilFaceNV) (GLenum);

	#endif // GL_NV_stencil_two_side_OGLEXT

	// - -[ gl_nv_vertex_array_range ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_vertex_array_range_OGLEXT

		GLvoid            (APIENTRY * m_pFlushVertexArrayRangeNV) ();
		GLvoid            (APIENTRY * m_pVertexArrayRangeNV) (GLsizei, GLvoid const *);

	#endif // GL_NV_vertex_array_range_OGLEXT

	// - -[ gl_nv_vertex_program ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NV_vertex_program_OGLEXT

		GLboolean         (APIENTRY * m_pAreProgramsResidentNV) (GLsizei, GLuint const *, GLboolean *);
		GLvoid            (APIENTRY * m_pBindProgramNV) (GLenum, GLuint);
		GLvoid            (APIENTRY * m_pDeleteProgramsNV) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pExecuteProgramNV) (GLenum, GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pGenProgramsNV) (GLsizei, GLuint *);
		GLvoid            (APIENTRY * m_pGetProgramivNV) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetProgramParameterdvNV) (GLenum, GLuint, GLenum, GLdouble *);
		GLvoid            (APIENTRY * m_pGetProgramParameterfvNV) (GLenum, GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetProgramStringNV) (GLuint, GLenum, GLubyte *);
		GLvoid            (APIENTRY * m_pGetTrackMatrixivNV) (GLenum, GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetVertexAttribdvNV) (GLuint, GLenum, GLdouble *);
		GLvoid            (APIENTRY * m_pGetVertexAttribfvNV) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetVertexAttribivNV) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetVertexAttribPointervNV) (GLuint, GLenum, GLvoid * *);
		GLboolean         (APIENTRY * m_pIsProgramNV) (GLuint);
		GLvoid            (APIENTRY * m_pLoadProgramNV) (GLenum, GLuint, GLsizei, GLubyte const *);
		GLvoid            (APIENTRY * m_pProgramParameter4dNV) (GLenum, GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pProgramParameter4dvNV) (GLenum, GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pProgramParameter4fNV) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pProgramParameter4fvNV) (GLenum, GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pProgramParameters4dvNV) (GLenum, GLuint, GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pProgramParameters4fvNV) (GLenum, GLuint, GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pRequestResidentProgramsNV) (GLsizei, GLuint const *);
		GLvoid            (APIENTRY * m_pTrackMatrixNV) (GLenum, GLuint, GLenum, GLenum);
		GLvoid            (APIENTRY * m_pVertexAttrib1dNV) (GLuint, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib1dvNV) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1fNV) (GLuint, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib1fvNV) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib1sNV) (GLuint, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib1svNV) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2dNV) (GLuint, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib2dvNV) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2fNV) (GLuint, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib2fvNV) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib2sNV) (GLuint, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib2svNV) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3dNV) (GLuint, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib3dvNV) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3fNV) (GLuint, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib3fvNV) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib3sNV) (GLuint, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib3svNV) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4dNV) (GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
		GLvoid            (APIENTRY * m_pVertexAttrib4dvNV) (GLuint, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4fNV) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pVertexAttrib4fvNV) (GLuint, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4sNV) (GLuint, GLshort, GLshort, GLshort, GLshort);
		GLvoid            (APIENTRY * m_pVertexAttrib4svNV) (GLuint, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttrib4ubNV) (GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
		GLvoid            (APIENTRY * m_pVertexAttrib4ubvNV) (GLuint, GLubyte const *);
		GLvoid            (APIENTRY * m_pVertexAttribPointerNV) (GLuint, GLint, GLenum, GLsizei, GLvoid const *);
		GLvoid            (APIENTRY * m_pVertexAttribs1dvNV) (GLuint, GLsizei, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttribs1fvNV) (GLuint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttribs1svNV) (GLuint, GLsizei, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttribs2dvNV) (GLuint, GLsizei, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttribs2fvNV) (GLuint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttribs2svNV) (GLuint, GLsizei, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttribs3dvNV) (GLuint, GLsizei, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttribs3fvNV) (GLuint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttribs3svNV) (GLuint, GLsizei, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttribs4dvNV) (GLuint, GLsizei, GLdouble const *);
		GLvoid            (APIENTRY * m_pVertexAttribs4fvNV) (GLuint, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pVertexAttribs4svNV) (GLuint, GLsizei, GLshort const *);
		GLvoid            (APIENTRY * m_pVertexAttribs4ubvNV) (GLuint, GLsizei, GLubyte const *);

	#endif // GL_NV_vertex_program_OGLEXT

	// - -[ gl_nvx_conditional_render ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_NVX_conditional_render_OGLEXT

		GLvoid            (APIENTRY * m_pBeginConditionalRenderNVX) (GLuint);
		GLvoid            (APIENTRY * m_pEndConditionalRenderNVX) ();

	#endif // GL_NVX_conditional_render_OGLEXT

	// - -[ gl_pgi_misc_hints ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_PGI_misc_hints_OGLEXT

		GLvoid            (APIENTRY * m_pHintPGI) (GLenum, GLint);

	#endif // GL_PGI_misc_hints_OGLEXT

	// - -[ gl_sgi_color_table ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGI_color_table_OGLEXT

		GLvoid            (APIENTRY * m_pColorTableParameterfvSGI) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pColorTableParameterivSGI) (GLenum, GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pColorTableSGI) (GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pCopyColorTableSGI) (GLenum, GLenum, GLint, GLint, GLsizei);
		GLvoid            (APIENTRY * m_pGetColorTableParameterfvSGI) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetColorTableParameterivSGI) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetColorTableSGI) (GLenum, GLenum, GLenum, GLvoid *);

	#endif // GL_SGI_color_table_OGLEXT

	// - -[ gl_sgis_detail_texture ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_detail_texture_OGLEXT

		GLvoid            (APIENTRY * m_pDetailTexFuncSGIS) (GLenum, GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pGetDetailTexFuncSGIS) (GLenum, GLfloat *);

	#endif // GL_SGIS_detail_texture_OGLEXT

	// - -[ gl_sgis_fog_function ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_fog_function_OGLEXT

		GLvoid            (APIENTRY * m_pFogFuncSGIS) (GLsizei, GLfloat const *);
		GLvoid            (APIENTRY * m_pGetFogFuncSGIS) (GLfloat *);

	#endif // GL_SGIS_fog_function_OGLEXT

	// - -[ gl_sgis_multisample ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_multisample_OGLEXT

		GLvoid            (APIENTRY * m_pSampleMaskSGIS) (GLclampf, GLboolean);
		GLvoid            (APIENTRY * m_pSamplePatternSGIS) (GLenum);

	#endif // GL_SGIS_multisample_OGLEXT

	// - -[ gl_sgis_pixel_texture ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_pixel_texture_OGLEXT

		GLvoid            (APIENTRY * m_pGetPixelTexGenParameterfvSGIS) (GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetPixelTexGenParameterivSGIS) (GLenum, GLint *);
		GLvoid            (APIENTRY * m_pPixelTexGenParameterfSGIS) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pPixelTexGenParameterfvSGIS) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pPixelTexGenParameteriSGIS) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pPixelTexGenParameterivSGIS) (GLenum, GLint const *);

	#endif // GL_SGIS_pixel_texture_OGLEXT

	// - -[ gl_sgis_point_parameters ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_point_parameters_OGLEXT

		GLvoid            (APIENTRY * m_pPointParameterfSGIS) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pPointParameterfvSGIS) (GLenum, GLfloat const *);

	#endif // GL_SGIS_point_parameters_OGLEXT

	// - -[ gl_sgis_sharpen_texture ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_sharpen_texture_OGLEXT

		GLvoid            (APIENTRY * m_pGetSharpenTexFuncSGIS) (GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pSharpenTexFuncSGIS) (GLenum, GLsizei, GLfloat const *);

	#endif // GL_SGIS_sharpen_texture_OGLEXT

	// - -[ gl_sgis_texture4d ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_texture4D_OGLEXT

		GLvoid            (APIENTRY * m_pTexImage4DSGIS) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *);
		GLvoid            (APIENTRY * m_pTexSubImage4DSGIS) (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLvoid const *);

	#endif // GL_SGIS_texture4D_OGLEXT

	// - -[ gl_sgis_texture_color_mask ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_texture_color_mask_OGLEXT

		GLvoid            (APIENTRY * m_pTextureColorMaskSGIS) (GLboolean, GLboolean, GLboolean, GLboolean);

	#endif // GL_SGIS_texture_color_mask_OGLEXT

	// - -[ gl_sgis_texture_filter4 ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIS_texture_filter4_OGLEXT

		GLvoid            (APIENTRY * m_pGetTexFilterFuncSGIS) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pTexFilterFuncSGIS) (GLenum, GLenum, GLsizei, GLfloat const *);

	#endif // GL_SGIS_texture_filter4_OGLEXT

	// - -[ gl_sgix_async ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_async_OGLEXT

		GLvoid            (APIENTRY * m_pAsyncMarkerSGIX) (GLuint);
		GLvoid            (APIENTRY * m_pDeleteAsyncMarkersSGIX) (GLuint, GLsizei);
		GLint             (APIENTRY * m_pFinishAsyncSGIX) (GLuint *);
		GLuint            (APIENTRY * m_pGenAsyncMarkersSGIX) (GLsizei);
		GLboolean         (APIENTRY * m_pIsAsyncMarkerSGIX) (GLuint);
		GLint             (APIENTRY * m_pPollAsyncSGIX) (GLuint *);

	#endif // GL_SGIX_async_OGLEXT

	// - -[ gl_sgix_flush_raster ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_flush_raster_OGLEXT

		GLvoid            (APIENTRY * m_pFlushRasterSGIX) ();

	#endif // GL_SGIX_flush_raster_OGLEXT

	// - -[ gl_sgix_fragment_lighting ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_fragment_lighting_OGLEXT

		GLvoid            (APIENTRY * m_pFragmentColorMaterialSGIX) (GLenum, GLenum);
		GLvoid            (APIENTRY * m_pFragmentLightfSGIX) (GLenum, GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pFragmentLightfvSGIX) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pFragmentLightiSGIX) (GLenum, GLenum, GLint);
		GLvoid            (APIENTRY * m_pFragmentLightivSGIX) (GLenum, GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pFragmentLightModelfSGIX) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pFragmentLightModelfvSGIX) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pFragmentLightModeliSGIX) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pFragmentLightModelivSGIX) (GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pFragmentMaterialfSGIX) (GLenum, GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pFragmentMaterialfvSGIX) (GLenum, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pFragmentMaterialiSGIX) (GLenum, GLenum, GLint);
		GLvoid            (APIENTRY * m_pFragmentMaterialivSGIX) (GLenum, GLenum, GLint const *);
		GLvoid            (APIENTRY * m_pGetFragmentLightfvSGIX) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetFragmentLightivSGIX) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pGetFragmentMaterialfvSGIX) (GLenum, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetFragmentMaterialivSGIX) (GLenum, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pLightEnviSGIX) (GLenum, GLint);

	#endif // GL_SGIX_fragment_lighting_OGLEXT

	// - -[ gl_sgix_framezoom ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_framezoom_OGLEXT

		GLvoid            (APIENTRY * m_pFrameZoomSGIX) (GLint);

	#endif // GL_SGIX_framezoom_OGLEXT

	// - -[ gl_sgix_igloo_interface ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_igloo_interface_OGLEXT

		GLvoid            (APIENTRY * m_pIglooInterfaceSGIX) (GLenum, GLvoid const *);

	#endif // GL_SGIX_igloo_interface_OGLEXT

	// - -[ gl_sgix_instruments ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_instruments_OGLEXT

		GLint             (APIENTRY * m_pGetInstrumentsSGIX) ();
		GLvoid            (APIENTRY * m_pInstrumentsBufferSGIX) (GLsizei, GLint *);
		GLint             (APIENTRY * m_pPollInstrumentsSGIX) (GLint *);
		GLvoid            (APIENTRY * m_pReadInstrumentsSGIX) (GLint);
		GLvoid            (APIENTRY * m_pStartInstrumentsSGIX) ();
		GLvoid            (APIENTRY * m_pStopInstrumentsSGIX) (GLint);

	#endif // GL_SGIX_instruments_OGLEXT

	// - -[ gl_sgix_list_priority ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_list_priority_OGLEXT

		GLvoid            (APIENTRY * m_pGetListParameterfvSGIX) (GLuint, GLenum, GLfloat *);
		GLvoid            (APIENTRY * m_pGetListParameterivSGIX) (GLuint, GLenum, GLint *);
		GLvoid            (APIENTRY * m_pListParameterfSGIX) (GLuint, GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pListParameterfvSGIX) (GLuint, GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pListParameteriSGIX) (GLuint, GLenum, GLint);
		GLvoid            (APIENTRY * m_pListParameterivSGIX) (GLuint, GLenum, GLint const *);

	#endif // GL_SGIX_list_priority_OGLEXT

	// - -[ gl_sgix_pixel_texture ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_pixel_texture_OGLEXT

		GLvoid            (APIENTRY * m_pPixelTexGenSGIX) (GLenum);

	#endif // GL_SGIX_pixel_texture_OGLEXT

	// - -[ gl_sgix_polynomial_ffd ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_polynomial_ffd_OGLEXT

		GLvoid            (APIENTRY * m_pDeformationMap3dSGIX) (GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, GLdouble const *);
		GLvoid            (APIENTRY * m_pDeformationMap3fSGIX) (GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat const *);
		GLvoid            (APIENTRY * m_pDeformSGIX) (GLbitfield);
		GLvoid            (APIENTRY * m_pLoadIdentityDeformationMapSGIX) (GLbitfield);

	#endif // GL_SGIX_polynomial_ffd_OGLEXT

	// - -[ gl_sgix_reference_plane ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_reference_plane_OGLEXT

		GLvoid            (APIENTRY * m_pReferencePlaneSGIX) (GLdouble const *);

	#endif // GL_SGIX_reference_plane_OGLEXT

	// - -[ gl_sgix_sprite ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_sprite_OGLEXT

		GLvoid            (APIENTRY * m_pSpriteParameterfSGIX) (GLenum, GLfloat);
		GLvoid            (APIENTRY * m_pSpriteParameterfvSGIX) (GLenum, GLfloat const *);
		GLvoid            (APIENTRY * m_pSpriteParameteriSGIX) (GLenum, GLint);
		GLvoid            (APIENTRY * m_pSpriteParameterivSGIX) (GLenum, GLint const *);

	#endif // GL_SGIX_sprite_OGLEXT

	// - -[ gl_sgix_tag_sample_buffer ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SGIX_tag_sample_buffer_OGLEXT

		GLvoid            (APIENTRY * m_pTagSampleBufferSGIX) ();

	#endif // GL_SGIX_tag_sample_buffer_OGLEXT

	// - -[ gl_sun_global_alpha ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SUN_global_alpha_OGLEXT

		GLvoid            (APIENTRY * m_pGlobalAlphaFactorbSUN) (GLbyte);
		GLvoid            (APIENTRY * m_pGlobalAlphaFactordSUN) (GLdouble);
		GLvoid            (APIENTRY * m_pGlobalAlphaFactorfSUN) (GLfloat);
		GLvoid            (APIENTRY * m_pGlobalAlphaFactoriSUN) (GLint);
		GLvoid            (APIENTRY * m_pGlobalAlphaFactorsSUN) (GLshort);
		GLvoid            (APIENTRY * m_pGlobalAlphaFactorubSUN) (GLubyte);
		GLvoid            (APIENTRY * m_pGlobalAlphaFactoruiSUN) (GLuint);
		GLvoid            (APIENTRY * m_pGlobalAlphaFactorusSUN) (GLushort);

	#endif // GL_SUN_global_alpha_OGLEXT

	// - -[ gl_sun_mesh_array ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SUN_mesh_array_OGLEXT

		GLvoid            (APIENTRY * m_pDrawMeshArraysSUN) (GLenum, GLint, GLsizei, GLsizei);

	#endif // GL_SUN_mesh_array_OGLEXT

	// - -[ gl_sun_triangle_list ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SUN_triangle_list_OGLEXT

		GLvoid            (APIENTRY * m_pReplacementCodePointerSUN) (GLenum, GLsizei, GLvoid const * *);
		GLvoid            (APIENTRY * m_pReplacementCodeubSUN) (GLubyte);
		GLvoid            (APIENTRY * m_pReplacementCodeubvSUN) (GLubyte const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiSUN) (GLuint);
		GLvoid            (APIENTRY * m_pReplacementCodeuivSUN) (GLuint const *);
		GLvoid            (APIENTRY * m_pReplacementCodeusSUN) (GLushort);
		GLvoid            (APIENTRY * m_pReplacementCodeusvSUN) (GLushort const *);

	#endif // GL_SUN_triangle_list_OGLEXT

	// - -[ gl_sun_vertex ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SUN_vertex_OGLEXT

		GLvoid            (APIENTRY * m_pColor3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pColor3fVertex3fvSUN) (GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pColor4fNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pColor4fNormal3fVertex3fvSUN) (GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pColor4ubVertex2fSUN) (GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pColor4ubVertex2fvSUN) (GLubyte const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pColor4ubVertex3fSUN) (GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pColor4ubVertex3fvSUN) (GLubyte const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pNormal3fVertex3fvSUN) (GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiColor3fVertex3fSUN) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiColor3fVertex3fvSUN) (GLuint const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiColor4fNormal3fVertex3fSUN) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiColor4fNormal3fVertex3fvSUN) (GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiColor4ubVertex3fSUN) (GLuint, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiColor4ubVertex3fvSUN) (GLuint const *, GLubyte const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiNormal3fVertex3fSUN) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiNormal3fVertex3fvSUN) (GLuint const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN) (GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiTexCoord2fNormal3fVertex3fSUN) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN) (GLuint const *, GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiTexCoord2fVertex3fSUN) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiTexCoord2fVertex3fvSUN) (GLuint const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pReplacementCodeuiVertex3fSUN) (GLuint, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pReplacementCodeuiVertex3fvSUN) (GLuint const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexCoord2fColor3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTexCoord2fColor3fVertex3fvSUN) (GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexCoord2fColor4fNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTexCoord2fColor4fNormal3fVertex3fvSUN) (GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexCoord2fColor4ubVertex3fSUN) (GLfloat, GLfloat, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTexCoord2fColor4ubVertex3fvSUN) (GLfloat const *, GLubyte const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexCoord2fNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTexCoord2fNormal3fVertex3fvSUN) (GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexCoord2fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTexCoord2fVertex3fvSUN) (GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexCoord4fColor4fNormal3fVertex4fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTexCoord4fColor4fNormal3fVertex4fvSUN) (GLfloat const *, GLfloat const *, GLfloat const *, GLfloat const *);
		GLvoid            (APIENTRY * m_pTexCoord4fVertex4fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
		GLvoid            (APIENTRY * m_pTexCoord4fVertex4fvSUN) (GLfloat const *, GLfloat const *);

	#endif // GL_SUN_vertex_OGLEXT

	// - -[ gl_sunx_constant_data ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef GL_SUNX_constant_data_OGLEXT

		GLvoid            (APIENTRY * m_pFinishTextureSUNX) ();

	#endif // GL_SUNX_constant_data_OGLEXT

};


// =============================================================================================================
// ===                                C L A S S   A D M I N I S T R A T I O N                                ===
// =============================================================================================================

//!	The destructor.

inline CRenderingContext::~CRenderingContext()
{
	// 1: traverse through the list and free all memory...

	SExtensionName * pExtensionName	= m_pFirstExtensionName;
	while(pExtensionName) {

		// 1.1: get the next extension name...

		SExtensionName * pOldExtensionName	= pExtensionName;
		pExtensionName								= pExtensionName->pNext;

		// 1.2: free the node memory...

		delete pOldExtensionName;
	}
}


// =============================================================================================================
// ===                              P R O T E C T E D   T O O L   M E T H O D S                              ===
// =============================================================================================================

//!	Init the list of extension names.

inline void CRenderingContext::InitExtensionString()
{
	// 1: initialize first pointer and get the opengl extension string...

	m_pFirstExtensionName = NULL;

	char const * szExtensions = (char const *) ::glGetString(GL_EXTENSIONS);
	if(!szExtensions) {

		return;
	}

	// 2: traverse through the extension string and build up the extension name list...

	do {

		// 2.1: create a new node and connect to the existing list...

		SExtensionName * pNewExtensionName	= new SExtensionName;

		pNewExtensionName->pNext				= m_pFirstExtensionName;
		pNewExtensionName->szExtension		= szExtensions;

		m_pFirstExtensionName					 = pNewExtensionName;

		// 2.2: skip the extension name...

		while((*szExtensions != ' ') && (*szExtensions != '\0')) {

			++szExtensions;
		}

		// 2.3: skip white spaces...

		while(*szExtensions == ' ') {

			++szExtensions;
		}
	}
	while(*szExtensions != '\0');
}

	
//!	Is list of extensions supported?

inline bool CRenderingContext::IsExtensionSupported(char const * szExtensions) const
{
	// 1: traverse through the list of given extension names...

	do {

		// 1.1: traverse through the list of supported extensions...

		SExtensionName * pExtensionName	= m_pFirstExtensionName;
		while(pExtensionName) {

			// 1.1.1: search for the end of one extension name, or a difference in names...

			char const * szWant	= szExtensions;
			char const * szThis	= pExtensionName->szExtension;

			while((*szThis != '\0') && (*szThis != ' ') && (*szWant == *szThis)) {

				++szWant;
				++szThis;
			}

			// 1.1.2: if we reached the end of the extension name, we continue with the next in the list...

			if(((*szWant == '\0') || (*szWant == ' ')) && ((*szThis == '\0') || (*szThis == ' '))) {

				break;
			}

			// 1.1.3: get the next supported extension name...

			pExtensionName = pExtensionName->pNext;
		}

		// 1.2: reached end of the supported extensions => specified extension is not supported...

		if(!pExtensionName) {

			return false;
		}

		// 1.3: skip extension name...

		while((*szExtensions != ' ') && (*szExtensions != '\0')) {

			++szExtensions;
		}

		// 1.4: skip whitespaces...

		while(*szExtensions == ' ') {

			++szExtensions;
		}
	}
	while(*szExtensions != '\0');

	// 2: all extensions supported...

	return true;
}


//!	Init the version id.

inline void CRenderingContext::InitVersionString()
{
	// 1: initialize version and get the opengl version string...

	m_uVersion = 0;

	char const * szMajorVersion = (char const *) ::glGetString(GL_VERSION);
	if(!szMajorVersion) {

		return;
	}

	// 2: extract the major version...

	m_uVersion |= atol(szMajorVersion) << 24;

	char const * szMinorVersion = strchr(szMajorVersion, '.');
	if(!szMinorVersion) {

		return;
	}

	// 3: extract the minor version...

	m_uVersion |= atol(szMinorVersion + 1) << 16;

	char const * szRevisionVersion = strchr(szMinorVersion + 1, '.');
	if(!szRevisionVersion) {

		return;
	}

	// 4: extract the release version...

	m_uVersion |= atol(szRevisionVersion + 1) << 0;
}

	
//!	Return the supported OpenGL version.

inline unsigned long CRenderingContext::GetVersion() const
{
	return m_uVersion;
}


#endif	// _OGL_RENDERINGCONTEXT_HPP_
