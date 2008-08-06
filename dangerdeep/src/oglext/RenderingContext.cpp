// RenderingContext.cpp                                      Copyright (C) 2006 Thomas Jansen (jansen@caesar.de)
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

#include	"RenderingContext.hpp"
#include	"Macros.h"
#include	"OglExt.h"

#include	<stdio.h>


// =============================================================================================================
// ===                                C L A S S   A D M I N I S T R A T I O N                                ===
// =============================================================================================================

//!	The constructor.

CRenderingContext::CRenderingContext()
{
	// 1: clear all uninitialized API function pointers...

	memset(this, 0, sizeof(CRenderingContext));

	// 2: init the extension strings...

	InitVersionString();
	InitExtensionString();

	// 3: init version...

	INIT_GLVERSION(                              1, 2, 0,                    InitVersion12);
	INIT_GLVERSION(                              1, 3, 0,                    InitVersion13);
	INIT_GLVERSION(                              1, 4, 0,                    InitVersion14);
	INIT_GLVERSION(                              1, 5, 0,                    InitVersion15);
	INIT_GLVERSION(                              2, 0, 0,                    InitVersion20);

	// 4: init extension...

	INIT_EXTENSION(                    "GL_3DFX_tbuffer",                  Init3dfxTbuffer);
	INIT_EXTENSION(             "GL_APPLE_element_array",            InitAppleElementArray);
	INIT_EXTENSION(                     "GL_APPLE_fence",                   InitAppleFence);
	INIT_EXTENSION(       "GL_APPLE_vertex_array_object",       InitAppleVertexArrayObject);
	INIT_EXTENSION(        "GL_APPLE_vertex_array_range",        InitAppleVertexArrayRange);
	INIT_EXTENSION(          "GL_ARB_color_buffer_float",          InitArbColorBufferFloat);
	INIT_EXTENSION(                "GL_ARB_draw_buffers",               InitArbDrawBuffers);
	INIT_EXTENSION(              "GL_ARB_matrix_palette",             InitArbMatrixPalette);
	INIT_EXTENSION(                 "GL_ARB_multisample",               InitArbMultisample);
	INIT_EXTENSION(                "GL_ARB_multitexture",              InitArbMultitexture);
	INIT_EXTENSION(             "GL_ARB_occlusion_query",            InitArbOcclusionQuery);
	INIT_EXTENSION(            "GL_ARB_point_parameters",           InitArbPointParameters);
	INIT_EXTENSION(              "GL_ARB_shader_objects",             InitArbShaderObjects);
	INIT_EXTENSION(         "GL_ARB_texture_compression",        InitArbTextureCompression);
	INIT_EXTENSION(            "GL_ARB_transpose_matrix",           InitArbTransposeMatrix);
	INIT_EXTENSION(                "GL_ARB_vertex_blend",               InitArbVertexBlend);
	INIT_EXTENSION(        "GL_ARB_vertex_buffer_object",        InitArbVertexBufferObject);
	INIT_EXTENSION(              "GL_ARB_vertex_program",             InitArbVertexProgram);
	INIT_EXTENSION(               "GL_ARB_vertex_shader",              InitArbVertexShader);
	INIT_EXTENSION(                  "GL_ARB_window_pos",                 InitArbWindowPos);
	INIT_EXTENSION(                "GL_ATI_draw_buffers",               InitAtiDrawBuffers);
	INIT_EXTENSION(               "GL_ATI_element_array",              InitAtiElementArray);
	INIT_EXTENSION(              "GL_ATI_envmap_bumpmap",             InitAtiEnvmapBumpmap);
	INIT_EXTENSION(             "GL_ATI_fragment_shader",            InitAtiFragmentShader);
	INIT_EXTENSION(           "GL_ATI_map_object_buffer",           InitAtiMapObjectBuffer);
	INIT_EXTENSION(                "GL_ATI_pn_triangles",               InitAtiPnTriangles);
	INIT_EXTENSION(            "GL_ATI_separate_stencil",           InitAtiSeparateStencil);
	INIT_EXTENSION(         "GL_ATI_vertex_array_object",         InitAtiVertexArrayObject);
	INIT_EXTENSION(  "GL_ATI_vertex_attrib_array_object",   InitAtiVertexAttribArrayObject);
	INIT_EXTENSION(              "GL_ATI_vertex_streams",             InitAtiVertexStreams);
	INIT_EXTENSION(                 "GL_EXT_blend_color",                InitExtBlendColor);
	INIT_EXTENSION(     "GL_EXT_blend_equation_separate",     InitExtBlendEquationSeparate);
	INIT_EXTENSION(         "GL_EXT_blend_func_separate",         InitExtBlendFuncSeparate);
	INIT_EXTENSION(                "GL_EXT_blend_minmax",               InitExtBlendMinmax);
	INIT_EXTENSION(              "GL_EXT_color_subtable",             InitExtColorSubtable);
	INIT_EXTENSION(       "GL_EXT_compiled_vertex_array",       InitExtCompiledVertexArray);
	INIT_EXTENSION(                 "GL_EXT_convolution",               InitExtConvolution);
	INIT_EXTENSION(            "GL_EXT_coordinate_frame",           InitExtCoordinateFrame);
	INIT_EXTENSION(                "GL_EXT_copy_texture",               InitExtCopyTexture);
	INIT_EXTENSION(                 "GL_EXT_cull_vertex",                InitExtCullVertex);
	INIT_EXTENSION(           "GL_EXT_depth_bounds_test",           InitExtDepthBoundsTest);
	INIT_EXTENSION(         "GL_EXT_draw_range_elements",         InitExtDrawRangeElements);
	INIT_EXTENSION(                   "GL_EXT_fog_coord",                  InitExtFogCoord);
	INIT_EXTENSION(          "GL_EXT_framebuffer_object",         InitExtFramebufferObject);
	INIT_EXTENSION(                   "GL_EXT_histogram",                 InitExtHistogram);
	INIT_EXTENSION(                  "GL_EXT_index_func",                 InitExtIndexFunc);
	INIT_EXTENSION(              "GL_EXT_index_material",             InitExtIndexMaterial);
	INIT_EXTENSION(               "GL_EXT_light_texture",              InitExtLightTexture);
	INIT_EXTENSION(           "GL_EXT_multi_draw_arrays",           InitExtMultiDrawArrays);
	INIT_EXTENSION(                 "GL_EXT_multisample",               InitExtMultisample);
	INIT_EXTENSION(            "GL_EXT_paletted_texture",           InitExtPalettedTexture);
	INIT_EXTENSION(             "GL_EXT_pixel_transform",            InitExtPixelTransform);
	INIT_EXTENSION(            "GL_EXT_point_parameters",           InitExtPointParameters);
	INIT_EXTENSION(              "GL_EXT_polygon_offset",             InitExtPolygonOffset);
	INIT_EXTENSION(             "GL_EXT_secondary_color",            InitExtSecondaryColor);
	INIT_EXTENSION(            "GL_EXT_stencil_two_side",            InitExtStencilTwoSide);
	INIT_EXTENSION(                  "GL_EXT_subtexture",                InitExtSubtexture);
	INIT_EXTENSION(                   "GL_EXT_texture3D",                 InitExtTexture3d);
	INIT_EXTENSION(              "GL_EXT_texture_object",             InitExtTextureObject);
	INIT_EXTENSION(      "GL_EXT_texture_perturb_normal",      InitExtTexturePerturbNormal);
	INIT_EXTENSION(                "GL_EXT_vertex_array",               InitExtVertexArray);
	INIT_EXTENSION(               "GL_EXT_vertex_shader",              InitExtVertexShader);
	INIT_EXTENSION(            "GL_EXT_vertex_weighting",           InitExtVertexWeighting);
	INIT_EXTENSION(           "GL_GREMEDY_string_marker",          InitGremedyStringMarker);
	INIT_EXTENSION(              "GL_HP_image_transform",             InitHpImageTransform);
	INIT_EXTENSION(       "GL_IBM_multimode_draw_arrays",       InitIbmMultimodeDrawArrays);
	INIT_EXTENSION(          "GL_IBM_vertex_array_lists",          InitIbmVertexArrayLists);
	INIT_EXTENSION(        "GL_INGR_blend_func_separate",        InitIngrBlendFuncSeparate);
	INIT_EXTENSION(           "GL_INTEL_parallel_arrays",          InitIntelParallelArrays);
	INIT_EXTENSION(             "GL_MESA_resize_buffers",            InitMesaResizeBuffers);
	INIT_EXTENSION(                 "GL_MESA_window_pos",                InitMesaWindowPos);
	INIT_EXTENSION(                "GL_NV_element_array",               InitNvElementArray);
	INIT_EXTENSION(                   "GL_NV_evaluators",                 InitNvEvaluators);
	INIT_EXTENSION(                        "GL_NV_fence",                      InitNvFence);
	INIT_EXTENSION(             "GL_NV_fragment_program",            InitNvFragmentProgram);
	INIT_EXTENSION(                   "GL_NV_half_float",                  InitNvHalfFloat);
	INIT_EXTENSION(              "GL_NV_occlusion_query",             InitNvOcclusionQuery);
	INIT_EXTENSION(             "GL_NV_pixel_data_range",             InitNvPixelDataRange);
	INIT_EXTENSION(                 "GL_NV_point_sprite",                InitNvPointSprite);
	INIT_EXTENSION(            "GL_NV_primitive_restart",           InitNvPrimitiveRestart);
	INIT_EXTENSION(           "GL_NV_register_combiners",          InitNvRegisterCombiners);
	INIT_EXTENSION(          "GL_NV_register_combiners2",         InitNvRegisterCombiners2);
	INIT_EXTENSION(             "GL_NV_stencil_two_side",             InitNvStencilTwoSide);
	INIT_EXTENSION(           "GL_NV_vertex_array_range",           InitNvVertexArrayRange);
	INIT_EXTENSION(               "GL_NV_vertex_program",              InitNvVertexProgram);
	INIT_EXTENSION(          "GL_NVX_conditional_render",         InitNvxConditionalRender);
	INIT_EXTENSION(                  "GL_PGI_misc_hints",                 InitPgiMiscHints);
	INIT_EXTENSION(                 "GL_SGI_color_table",                InitSgiColorTable);
	INIT_EXTENSION(             "GL_SGIS_detail_texture",            InitSgisDetailTexture);
	INIT_EXTENSION(               "GL_SGIS_fog_function",              InitSgisFogFunction);
	INIT_EXTENSION(                "GL_SGIS_multisample",              InitSgisMultisample);
	INIT_EXTENSION(              "GL_SGIS_pixel_texture",             InitSgisPixelTexture);
	INIT_EXTENSION(           "GL_SGIS_point_parameters",          InitSgisPointParameters);
	INIT_EXTENSION(            "GL_SGIS_sharpen_texture",           InitSgisSharpenTexture);
	INIT_EXTENSION(                  "GL_SGIS_texture4D",                InitSgisTexture4d);
	INIT_EXTENSION(         "GL_SGIS_texture_color_mask",         InitSgisTextureColorMask);
	INIT_EXTENSION(            "GL_SGIS_texture_filter4",           InitSgisTextureFilter4);
	INIT_EXTENSION(                      "GL_SGIX_async",                    InitSgixAsync);
	INIT_EXTENSION(               "GL_SGIX_flush_raster",              InitSgixFlushRaster);
	INIT_EXTENSION(          "GL_SGIX_fragment_lighting",         InitSgixFragmentLighting);
	INIT_EXTENSION(                  "GL_SGIX_framezoom",                InitSgixFramezoom);
	INIT_EXTENSION(            "GL_SGIX_igloo_interface",           InitSgixIglooInterface);
	INIT_EXTENSION(                "GL_SGIX_instruments",              InitSgixInstruments);
	INIT_EXTENSION(              "GL_SGIX_list_priority",             InitSgixListPriority);
	INIT_EXTENSION(              "GL_SGIX_pixel_texture",             InitSgixPixelTexture);
	INIT_EXTENSION(             "GL_SGIX_polynomial_ffd",            InitSgixPolynomialFfd);
	INIT_EXTENSION(            "GL_SGIX_reference_plane",           InitSgixReferencePlane);
	INIT_EXTENSION(                     "GL_SGIX_sprite",                   InitSgixSprite);
	INIT_EXTENSION(          "GL_SGIX_tag_sample_buffer",          InitSgixTagSampleBuffer);
	INIT_EXTENSION(                "GL_SUN_global_alpha",               InitSunGlobalAlpha);
	INIT_EXTENSION(                  "GL_SUN_mesh_array",                 InitSunMeshArray);
	INIT_EXTENSION(               "GL_SUN_triangle_list",              InitSunTriangleList);
	INIT_EXTENSION(                      "GL_SUN_vertex",                    InitSunVertex);
	INIT_EXTENSION(              "GL_SUNX_constant_data",             InitSunxConstantData);
}



// =============================================================================================================
// ===                                    S T A T I C   F U N C T I O N S                                    ===
// =============================================================================================================

//!	Return address of a given OpenGL Function.

void * CRenderingContext::GetProcAddress(char const * szFunction)
{
	#if defined(_WIN32)
	
		// W: implementation for microsoft windows...

			// W.1: create a new function name...

			char * szNewFunction = new char [strlen(szFunction) + 3];
			if(!szNewFunction) {

				return NULL;
			}

			// W.2: create the new function name...
		
			szNewFunction[0]	= 'g';
			szNewFunction[1]	= 'l';
			strcpy(&szNewFunction[2], szFunction);

			// W.3: determine the address...

            // TJ: casted to avoid compiler error. Ugly....
			void * pProcAddress = (void*) ::wglGetProcAddress(szNewFunction);

			delete [] szNewFunction;

			return pProcAddress;

	#elif (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)

		// M: implementation for apple mac os x...

			// M.1: create a new function name...

			char * szNewFunction = new char [strlen(szFunction) + 4];
			if(!szNewFunction) {

				return NULL;
			}

			// M.2: create the new function name...

			szNewFunction[0]	= '_';
			szNewFunction[1]	= 'g';
			szNewFunction[2]	= 'l';
			strcpy(&szNewFunction[3], szFunction);

			// M.3: check if the symbol name is defined...

			if(!NSIsSymbolNameDefined(szNewFunction)) {

				delete [] szNewFunction;
				return NULL;
			}

			// M.4: look-up and bind the symbol...

			NSSymbol symProcedure = NSLookupAndBindSymbol(szNewFunction);
			if(!symProcedure) {

				delete [] szNewFunction;
				return NULL;
			}

			// M.5: determine the address...

			void * pProcAddress = NSAddressOfSymbol(symProcedure);

			delete [] szNewFunction;

			return pProcAddress;

	#else

		// U: implementation for broad range of UNIXes...

			// U.1: create a new function name...

			char * szNewFunction = new char [strlen(szFunction) + 3];
			if(!szNewFunction) {

				return NULL;
			}

			// U.2: create the new function name...
		
			szNewFunction[0]	= 'g';
			szNewFunction[1]	= 'l';
			strcpy(&szNewFunction[2], szFunction);

			// U.3: determine the address...

			#ifndef	GLX_GLXEXT_LEGACY			

				void * pProcAddress = (void *) ::glXGetProcAddress((GLubyte const *) szNewFunction);

			#else
			
				void * pProcAddress = (void *) ::glXGetProcAddressARB((GLubyte const *) szNewFunction);	
			
			#endif

			delete [] szNewFunction;

			return pProcAddress;
	
	#endif
}



/* ========================================================================================================== */
/* ===                                           V E R S I O N                                            === */
/* ========================================================================================================== */

// ---[ GL_VERSION_1_2 ]----------------------------------------------------------------------------------------

//!	Initialize GL_VERSION_1_2.

bool CRenderingContext::InitVersion12()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_2_OGLEXT

		GET_PROC_ADDRESS(BlendColor);
		GET_PROC_ADDRESS(BlendEquation);
		GET_PROC_ADDRESS(ColorSubTable);
		GET_PROC_ADDRESS(ColorTable);
		GET_PROC_ADDRESS(ColorTableParameterfv);
		GET_PROC_ADDRESS(ColorTableParameteriv);
		GET_PROC_ADDRESS(ConvolutionFilter1D);
		GET_PROC_ADDRESS(ConvolutionFilter2D);
		GET_PROC_ADDRESS(ConvolutionParameterf);
		GET_PROC_ADDRESS(ConvolutionParameterfv);
		GET_PROC_ADDRESS(ConvolutionParameteri);
		GET_PROC_ADDRESS(ConvolutionParameteriv);
		GET_PROC_ADDRESS(CopyColorSubTable);
		GET_PROC_ADDRESS(CopyColorTable);
		GET_PROC_ADDRESS(CopyConvolutionFilter1D);
		GET_PROC_ADDRESS(CopyConvolutionFilter2D);
		GET_PROC_ADDRESS(CopyTexSubImage3D);
		GET_PROC_ADDRESS(DrawRangeElements);
		GET_PROC_ADDRESS(GetColorTable);
		GET_PROC_ADDRESS(GetColorTableParameterfv);
		GET_PROC_ADDRESS(GetColorTableParameteriv);
		GET_PROC_ADDRESS(GetConvolutionFilter);
		GET_PROC_ADDRESS(GetConvolutionParameterfv);
		GET_PROC_ADDRESS(GetConvolutionParameteriv);
		GET_PROC_ADDRESS(GetHistogram);
		GET_PROC_ADDRESS(GetHistogramParameterfv);
		GET_PROC_ADDRESS(GetHistogramParameteriv);
		GET_PROC_ADDRESS(GetMinmax);
		GET_PROC_ADDRESS(GetMinmaxParameterfv);
		GET_PROC_ADDRESS(GetMinmaxParameteriv);
		GET_PROC_ADDRESS(GetSeparableFilter);
		GET_PROC_ADDRESS(Histogram);
		GET_PROC_ADDRESS(Minmax);
		GET_PROC_ADDRESS(ResetHistogram);
		GET_PROC_ADDRESS(ResetMinmax);
		GET_PROC_ADDRESS(SeparableFilter2D);
		GET_PROC_ADDRESS(TexImage3D);
		GET_PROC_ADDRESS(TexSubImage3D);

	#endif // GL_VERSION_1_2_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_VERSION_1_3 ]----------------------------------------------------------------------------------------

//!	Initialize GL_VERSION_1_3.

bool CRenderingContext::InitVersion13()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_3_OGLEXT

		GET_PROC_ADDRESS(ActiveTexture);
		GET_PROC_ADDRESS(ClientActiveTexture);
		GET_PROC_ADDRESS(CompressedTexImage1D);
		GET_PROC_ADDRESS(CompressedTexImage2D);
		GET_PROC_ADDRESS(CompressedTexImage3D);
		GET_PROC_ADDRESS(CompressedTexSubImage1D);
		GET_PROC_ADDRESS(CompressedTexSubImage2D);
		GET_PROC_ADDRESS(CompressedTexSubImage3D);
		GET_PROC_ADDRESS(GetCompressedTexImage);
		GET_PROC_ADDRESS(LoadTransposeMatrixd);
		GET_PROC_ADDRESS(LoadTransposeMatrixf);
		GET_PROC_ADDRESS(MultiTexCoord1d);
		GET_PROC_ADDRESS(MultiTexCoord1dv);
		GET_PROC_ADDRESS(MultiTexCoord1f);
		GET_PROC_ADDRESS(MultiTexCoord1fv);
		GET_PROC_ADDRESS(MultiTexCoord1i);
		GET_PROC_ADDRESS(MultiTexCoord1iv);
		GET_PROC_ADDRESS(MultiTexCoord1s);
		GET_PROC_ADDRESS(MultiTexCoord1sv);
		GET_PROC_ADDRESS(MultiTexCoord2d);
		GET_PROC_ADDRESS(MultiTexCoord2dv);
		GET_PROC_ADDRESS(MultiTexCoord2f);
		GET_PROC_ADDRESS(MultiTexCoord2fv);
		GET_PROC_ADDRESS(MultiTexCoord2i);
		GET_PROC_ADDRESS(MultiTexCoord2iv);
		GET_PROC_ADDRESS(MultiTexCoord2s);
		GET_PROC_ADDRESS(MultiTexCoord2sv);
		GET_PROC_ADDRESS(MultiTexCoord3d);
		GET_PROC_ADDRESS(MultiTexCoord3dv);
		GET_PROC_ADDRESS(MultiTexCoord3f);
		GET_PROC_ADDRESS(MultiTexCoord3fv);
		GET_PROC_ADDRESS(MultiTexCoord3i);
		GET_PROC_ADDRESS(MultiTexCoord3iv);
		GET_PROC_ADDRESS(MultiTexCoord3s);
		GET_PROC_ADDRESS(MultiTexCoord3sv);
		GET_PROC_ADDRESS(MultiTexCoord4d);
		GET_PROC_ADDRESS(MultiTexCoord4dv);
		GET_PROC_ADDRESS(MultiTexCoord4f);
		GET_PROC_ADDRESS(MultiTexCoord4fv);
		GET_PROC_ADDRESS(MultiTexCoord4i);
		GET_PROC_ADDRESS(MultiTexCoord4iv);
		GET_PROC_ADDRESS(MultiTexCoord4s);
		GET_PROC_ADDRESS(MultiTexCoord4sv);
		GET_PROC_ADDRESS(MultTransposeMatrixd);
		GET_PROC_ADDRESS(MultTransposeMatrixf);
		GET_PROC_ADDRESS(SampleCoverage);

	#endif // GL_VERSION_1_3_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_VERSION_1_4 ]----------------------------------------------------------------------------------------

//!	Initialize GL_VERSION_1_4.

bool CRenderingContext::InitVersion14()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_4_OGLEXT

		GET_PROC_ADDRESS(BlendFuncSeparate);
		GET_PROC_ADDRESS(FogCoordd);
		GET_PROC_ADDRESS(FogCoorddv);
		GET_PROC_ADDRESS(FogCoordf);
		GET_PROC_ADDRESS(FogCoordfv);
		GET_PROC_ADDRESS(FogCoordPointer);
		GET_PROC_ADDRESS(MultiDrawArrays);
		GET_PROC_ADDRESS(MultiDrawElements);
		GET_PROC_ADDRESS(PointParameterf);
		GET_PROC_ADDRESS(PointParameterfv);
		GET_PROC_ADDRESS(PointParameteri);
		GET_PROC_ADDRESS(PointParameteriv);
		GET_PROC_ADDRESS(SecondaryColor3b);
		GET_PROC_ADDRESS(SecondaryColor3bv);
		GET_PROC_ADDRESS(SecondaryColor3d);
		GET_PROC_ADDRESS(SecondaryColor3dv);
		GET_PROC_ADDRESS(SecondaryColor3f);
		GET_PROC_ADDRESS(SecondaryColor3fv);
		GET_PROC_ADDRESS(SecondaryColor3i);
		GET_PROC_ADDRESS(SecondaryColor3iv);
		GET_PROC_ADDRESS(SecondaryColor3s);
		GET_PROC_ADDRESS(SecondaryColor3sv);
		GET_PROC_ADDRESS(SecondaryColor3ub);
		GET_PROC_ADDRESS(SecondaryColor3ubv);
		GET_PROC_ADDRESS(SecondaryColor3ui);
		GET_PROC_ADDRESS(SecondaryColor3uiv);
		GET_PROC_ADDRESS(SecondaryColor3us);
		GET_PROC_ADDRESS(SecondaryColor3usv);
		GET_PROC_ADDRESS(SecondaryColorPointer);
		GET_PROC_ADDRESS(WindowPos2d);
		GET_PROC_ADDRESS(WindowPos2dv);
		GET_PROC_ADDRESS(WindowPos2f);
		GET_PROC_ADDRESS(WindowPos2fv);
		GET_PROC_ADDRESS(WindowPos2i);
		GET_PROC_ADDRESS(WindowPos2iv);
		GET_PROC_ADDRESS(WindowPos2s);
		GET_PROC_ADDRESS(WindowPos2sv);
		GET_PROC_ADDRESS(WindowPos3d);
		GET_PROC_ADDRESS(WindowPos3dv);
		GET_PROC_ADDRESS(WindowPos3f);
		GET_PROC_ADDRESS(WindowPos3fv);
		GET_PROC_ADDRESS(WindowPos3i);
		GET_PROC_ADDRESS(WindowPos3iv);
		GET_PROC_ADDRESS(WindowPos3s);
		GET_PROC_ADDRESS(WindowPos3sv);

	#endif // GL_VERSION_1_4_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_VERSION_1_5 ]----------------------------------------------------------------------------------------

//!	Initialize GL_VERSION_1_5.

bool CRenderingContext::InitVersion15()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_5_OGLEXT

		GET_PROC_ADDRESS(BeginQuery);
		GET_PROC_ADDRESS(BindBuffer);
		GET_PROC_ADDRESS(BufferData);
		GET_PROC_ADDRESS(BufferSubData);
		GET_PROC_ADDRESS(DeleteBuffers);
		GET_PROC_ADDRESS(DeleteQueries);
		GET_PROC_ADDRESS(EndQuery);
		GET_PROC_ADDRESS(GenBuffers);
		GET_PROC_ADDRESS(GenQueries);
		GET_PROC_ADDRESS(GetBufferParameteriv);
		GET_PROC_ADDRESS(GetBufferPointerv);
		GET_PROC_ADDRESS(GetBufferSubData);
		GET_PROC_ADDRESS(GetQueryiv);
		GET_PROC_ADDRESS(GetQueryObjectiv);
		GET_PROC_ADDRESS(GetQueryObjectuiv);
		GET_PROC_ADDRESS(IsBuffer);
		GET_PROC_ADDRESS(IsQuery);
		GET_PROC_ADDRESS(MapBuffer);
		GET_PROC_ADDRESS(UnmapBuffer);

	#endif // GL_VERSION_1_5_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_VERSION_2_0 ]----------------------------------------------------------------------------------------

//!	Initialize GL_VERSION_2_0.

bool CRenderingContext::InitVersion20()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_2_0_OGLEXT

		GET_PROC_ADDRESS(AttachShader);
		GET_PROC_ADDRESS(BindAttribLocation);
		GET_PROC_ADDRESS(BlendEquationSeparate);
		GET_PROC_ADDRESS(CompileShader);
		GET_PROC_ADDRESS(CreateProgram);
		GET_PROC_ADDRESS(CreateShader);
		GET_PROC_ADDRESS(DeleteProgram);
		GET_PROC_ADDRESS(DeleteShader);
		GET_PROC_ADDRESS(DetachShader);
		GET_PROC_ADDRESS(DisableVertexAttribArray);
		GET_PROC_ADDRESS(DrawBuffers);
		GET_PROC_ADDRESS(EnableVertexAttribArray);
		GET_PROC_ADDRESS(GetActiveAttrib);
		GET_PROC_ADDRESS(GetActiveUniform);
		GET_PROC_ADDRESS(GetAttachedShaders);
		GET_PROC_ADDRESS(GetAttribLocation);
		GET_PROC_ADDRESS(GetProgramInfoLog);
		GET_PROC_ADDRESS(GetProgramiv);
		GET_PROC_ADDRESS(GetShaderInfoLog);
		GET_PROC_ADDRESS(GetShaderiv);
		GET_PROC_ADDRESS(GetShaderSource);
		GET_PROC_ADDRESS(GetUniformfv);
		GET_PROC_ADDRESS(GetUniformiv);
		GET_PROC_ADDRESS(GetUniformLocation);
		GET_PROC_ADDRESS(GetVertexAttribdv);
		GET_PROC_ADDRESS(GetVertexAttribfv);
		GET_PROC_ADDRESS(GetVertexAttribiv);
		GET_PROC_ADDRESS(GetVertexAttribPointerv);
		GET_PROC_ADDRESS(IsProgram);
		GET_PROC_ADDRESS(IsShader);
		GET_PROC_ADDRESS(LinkProgram);
		GET_PROC_ADDRESS(ShaderSource);
		GET_PROC_ADDRESS(StencilFuncSeparate);
		GET_PROC_ADDRESS(StencilMaskSeparate);
		GET_PROC_ADDRESS(StencilOpSeparate);
		GET_PROC_ADDRESS(Uniform1f);
		GET_PROC_ADDRESS(Uniform1fv);
		GET_PROC_ADDRESS(Uniform1i);
		GET_PROC_ADDRESS(Uniform1iv);
		GET_PROC_ADDRESS(Uniform2f);
		GET_PROC_ADDRESS(Uniform2fv);
		GET_PROC_ADDRESS(Uniform2i);
		GET_PROC_ADDRESS(Uniform2iv);
		GET_PROC_ADDRESS(Uniform3f);
		GET_PROC_ADDRESS(Uniform3fv);
		GET_PROC_ADDRESS(Uniform3i);
		GET_PROC_ADDRESS(Uniform3iv);
		GET_PROC_ADDRESS(Uniform4f);
		GET_PROC_ADDRESS(Uniform4fv);
		GET_PROC_ADDRESS(Uniform4i);
		GET_PROC_ADDRESS(Uniform4iv);
		GET_PROC_ADDRESS(UniformMatrix2fv);
		GET_PROC_ADDRESS(UniformMatrix3fv);
		GET_PROC_ADDRESS(UniformMatrix4fv);
		GET_PROC_ADDRESS(UseProgram);
		GET_PROC_ADDRESS(ValidateProgram);
		GET_PROC_ADDRESS(VertexAttrib1d);
		GET_PROC_ADDRESS(VertexAttrib1dv);
		GET_PROC_ADDRESS(VertexAttrib1f);
		GET_PROC_ADDRESS(VertexAttrib1fv);
		GET_PROC_ADDRESS(VertexAttrib1s);
		GET_PROC_ADDRESS(VertexAttrib1sv);
		GET_PROC_ADDRESS(VertexAttrib2d);
		GET_PROC_ADDRESS(VertexAttrib2dv);
		GET_PROC_ADDRESS(VertexAttrib2f);
		GET_PROC_ADDRESS(VertexAttrib2fv);
		GET_PROC_ADDRESS(VertexAttrib2s);
		GET_PROC_ADDRESS(VertexAttrib2sv);
		GET_PROC_ADDRESS(VertexAttrib3d);
		GET_PROC_ADDRESS(VertexAttrib3dv);
		GET_PROC_ADDRESS(VertexAttrib3f);
		GET_PROC_ADDRESS(VertexAttrib3fv);
		GET_PROC_ADDRESS(VertexAttrib3s);
		GET_PROC_ADDRESS(VertexAttrib3sv);
		GET_PROC_ADDRESS(VertexAttrib4bv);
		GET_PROC_ADDRESS(VertexAttrib4d);
		GET_PROC_ADDRESS(VertexAttrib4dv);
		GET_PROC_ADDRESS(VertexAttrib4f);
		GET_PROC_ADDRESS(VertexAttrib4fv);
		GET_PROC_ADDRESS(VertexAttrib4iv);
		GET_PROC_ADDRESS(VertexAttrib4Nbv);
		GET_PROC_ADDRESS(VertexAttrib4Niv);
		GET_PROC_ADDRESS(VertexAttrib4Nsv);
		GET_PROC_ADDRESS(VertexAttrib4Nub);
		GET_PROC_ADDRESS(VertexAttrib4Nubv);
		GET_PROC_ADDRESS(VertexAttrib4Nuiv);
		GET_PROC_ADDRESS(VertexAttrib4Nusv);
		GET_PROC_ADDRESS(VertexAttrib4s);
		GET_PROC_ADDRESS(VertexAttrib4sv);
		GET_PROC_ADDRESS(VertexAttrib4ubv);
		GET_PROC_ADDRESS(VertexAttrib4uiv);
		GET_PROC_ADDRESS(VertexAttrib4usv);
		GET_PROC_ADDRESS(VertexAttribPointer);

	#endif // GL_VERSION_2_0_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


/* ========================================================================================================== */
/* ===                                         E X T E N S I O N                                          === */
/* ========================================================================================================== */

// ---[ GL_3DFX_tbuffer ]---------------------------------------------------------------------------------------

//!	Initialize GL_3DFX_tbuffer.

bool CRenderingContext::Init3dfxTbuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_3DFX_tbuffer_OGLEXT

		GET_PROC_ADDRESS(TbufferMask3DFX);

	#endif // GL_3DFX_tbuffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_element_array ]--------------------------------------------------------------------------------

//!	Initialize GL_APPLE_element_array.

bool CRenderingContext::InitAppleElementArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_element_array_OGLEXT

		GET_PROC_ADDRESS(DrawElementArrayAPPLE);
		GET_PROC_ADDRESS(DrawRangeElementArrayAPPLE);
		GET_PROC_ADDRESS(ElementPointerAPPLE);
		GET_PROC_ADDRESS(MultiDrawElementArrayAPPLE);
		GET_PROC_ADDRESS(MultiDrawRangeElementArrayAPPLE);

	#endif // GL_APPLE_element_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_fence ]----------------------------------------------------------------------------------------

//!	Initialize GL_APPLE_fence.

bool CRenderingContext::InitAppleFence()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_fence_OGLEXT

		GET_PROC_ADDRESS(DeleteFencesAPPLE);
		GET_PROC_ADDRESS(FinishFenceAPPLE);
		GET_PROC_ADDRESS(FinishObjectAPPLE);
		GET_PROC_ADDRESS(GenFencesAPPLE);
		GET_PROC_ADDRESS(IsFenceAPPLE);
		GET_PROC_ADDRESS(SetFenceAPPLE);
		GET_PROC_ADDRESS(TestFenceAPPLE);
		GET_PROC_ADDRESS(TestObjectAPPLE);

	#endif // GL_APPLE_fence_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_vertex_array_object ]--------------------------------------------------------------------------

//!	Initialize GL_APPLE_vertex_array_object.

bool CRenderingContext::InitAppleVertexArrayObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_vertex_array_object_OGLEXT

		GET_PROC_ADDRESS(BindVertexArrayAPPLE);
		GET_PROC_ADDRESS(DeleteVertexArraysAPPLE);
		GET_PROC_ADDRESS(GenVertexArraysAPPLE);
		GET_PROC_ADDRESS(IsVertexArrayAPPLE);

	#endif // GL_APPLE_vertex_array_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_vertex_array_range ]---------------------------------------------------------------------------

//!	Initialize GL_APPLE_vertex_array_range.

bool CRenderingContext::InitAppleVertexArrayRange()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_vertex_array_range_OGLEXT

		GET_PROC_ADDRESS(FlushVertexArrayRangeAPPLE);
		GET_PROC_ADDRESS(VertexArrayParameteriAPPLE);
		GET_PROC_ADDRESS(VertexArrayRangeAPPLE);

	#endif // GL_APPLE_vertex_array_range_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_color_buffer_float ]-----------------------------------------------------------------------------

//!	Initialize GL_ARB_color_buffer_float.

bool CRenderingContext::InitArbColorBufferFloat()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_color_buffer_float_OGLEXT

		GET_PROC_ADDRESS(ClampColorARB);

	#endif // GL_ARB_color_buffer_float_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_draw_buffers ]-----------------------------------------------------------------------------------

//!	Initialize GL_ARB_draw_buffers.

bool CRenderingContext::InitArbDrawBuffers()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_draw_buffers_OGLEXT

		GET_PROC_ADDRESS(DrawBuffersARB);

	#endif // GL_ARB_draw_buffers_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_matrix_palette ]---------------------------------------------------------------------------------

//!	Initialize GL_ARB_matrix_palette.

bool CRenderingContext::InitArbMatrixPalette()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_matrix_palette_OGLEXT

		GET_PROC_ADDRESS(CurrentPaletteMatrixARB);
		GET_PROC_ADDRESS(MatrixIndexPointerARB);
		GET_PROC_ADDRESS(MatrixIndexubvARB);
		GET_PROC_ADDRESS(MatrixIndexuivARB);
		GET_PROC_ADDRESS(MatrixIndexusvARB);

	#endif // GL_ARB_matrix_palette_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_multisample ]------------------------------------------------------------------------------------

//!	Initialize GL_ARB_multisample.

bool CRenderingContext::InitArbMultisample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_multisample_OGLEXT

		GET_PROC_ADDRESS(SampleCoverageARB);

	#endif // GL_ARB_multisample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_multitexture ]-----------------------------------------------------------------------------------

//!	Initialize GL_ARB_multitexture.

bool CRenderingContext::InitArbMultitexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_multitexture_OGLEXT

		GET_PROC_ADDRESS(ActiveTextureARB);
		GET_PROC_ADDRESS(ClientActiveTextureARB);
		GET_PROC_ADDRESS(MultiTexCoord1dARB);
		GET_PROC_ADDRESS(MultiTexCoord1dvARB);
		GET_PROC_ADDRESS(MultiTexCoord1fARB);
		GET_PROC_ADDRESS(MultiTexCoord1fvARB);
		GET_PROC_ADDRESS(MultiTexCoord1iARB);
		GET_PROC_ADDRESS(MultiTexCoord1ivARB);
		GET_PROC_ADDRESS(MultiTexCoord1sARB);
		GET_PROC_ADDRESS(MultiTexCoord1svARB);
		GET_PROC_ADDRESS(MultiTexCoord2dARB);
		GET_PROC_ADDRESS(MultiTexCoord2dvARB);
		GET_PROC_ADDRESS(MultiTexCoord2fARB);
		GET_PROC_ADDRESS(MultiTexCoord2fvARB);
		GET_PROC_ADDRESS(MultiTexCoord2iARB);
		GET_PROC_ADDRESS(MultiTexCoord2ivARB);
		GET_PROC_ADDRESS(MultiTexCoord2sARB);
		GET_PROC_ADDRESS(MultiTexCoord2svARB);
		GET_PROC_ADDRESS(MultiTexCoord3dARB);
		GET_PROC_ADDRESS(MultiTexCoord3dvARB);
		GET_PROC_ADDRESS(MultiTexCoord3fARB);
		GET_PROC_ADDRESS(MultiTexCoord3fvARB);
		GET_PROC_ADDRESS(MultiTexCoord3iARB);
		GET_PROC_ADDRESS(MultiTexCoord3ivARB);
		GET_PROC_ADDRESS(MultiTexCoord3sARB);
		GET_PROC_ADDRESS(MultiTexCoord3svARB);
		GET_PROC_ADDRESS(MultiTexCoord4dARB);
		GET_PROC_ADDRESS(MultiTexCoord4dvARB);
		GET_PROC_ADDRESS(MultiTexCoord4fARB);
		GET_PROC_ADDRESS(MultiTexCoord4fvARB);
		GET_PROC_ADDRESS(MultiTexCoord4iARB);
		GET_PROC_ADDRESS(MultiTexCoord4ivARB);
		GET_PROC_ADDRESS(MultiTexCoord4sARB);
		GET_PROC_ADDRESS(MultiTexCoord4svARB);

	#endif // GL_ARB_multitexture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_occlusion_query ]--------------------------------------------------------------------------------

//!	Initialize GL_ARB_occlusion_query.

bool CRenderingContext::InitArbOcclusionQuery()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_occlusion_query_OGLEXT

		GET_PROC_ADDRESS(BeginQueryARB);
		GET_PROC_ADDRESS(DeleteQueriesARB);
		GET_PROC_ADDRESS(EndQueryARB);
		GET_PROC_ADDRESS(GenQueriesARB);
		GET_PROC_ADDRESS(GetQueryivARB);
		GET_PROC_ADDRESS(GetQueryObjectivARB);
		GET_PROC_ADDRESS(GetQueryObjectuivARB);
		GET_PROC_ADDRESS(IsQueryARB);

	#endif // GL_ARB_occlusion_query_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_point_parameters ]-------------------------------------------------------------------------------

//!	Initialize GL_ARB_point_parameters.

bool CRenderingContext::InitArbPointParameters()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_point_parameters_OGLEXT

		GET_PROC_ADDRESS(PointParameterfARB);
		GET_PROC_ADDRESS(PointParameterfvARB);

	#endif // GL_ARB_point_parameters_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_shader_objects ]---------------------------------------------------------------------------------

//!	Initialize GL_ARB_shader_objects.

bool CRenderingContext::InitArbShaderObjects()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_shader_objects_OGLEXT

		GET_PROC_ADDRESS(AttachObjectARB);
		GET_PROC_ADDRESS(CompileShaderARB);
		GET_PROC_ADDRESS(CreateProgramObjectARB);
		GET_PROC_ADDRESS(CreateShaderObjectARB);
		GET_PROC_ADDRESS(DeleteObjectARB);
		GET_PROC_ADDRESS(DetachObjectARB);
		GET_PROC_ADDRESS(GetActiveUniformARB);
		GET_PROC_ADDRESS(GetAttachedObjectsARB);
		GET_PROC_ADDRESS(GetHandleARB);
		GET_PROC_ADDRESS(GetInfoLogARB);
		GET_PROC_ADDRESS(GetObjectParameterfvARB);
		GET_PROC_ADDRESS(GetObjectParameterivARB);
		GET_PROC_ADDRESS(GetShaderSourceARB);
		GET_PROC_ADDRESS(GetUniformfvARB);
		GET_PROC_ADDRESS(GetUniformivARB);
		GET_PROC_ADDRESS(GetUniformLocationARB);
		GET_PROC_ADDRESS(LinkProgramARB);
		GET_PROC_ADDRESS(ShaderSourceARB);
		GET_PROC_ADDRESS(Uniform1fARB);
		GET_PROC_ADDRESS(Uniform1fvARB);
		GET_PROC_ADDRESS(Uniform1iARB);
		GET_PROC_ADDRESS(Uniform1ivARB);
		GET_PROC_ADDRESS(Uniform2fARB);
		GET_PROC_ADDRESS(Uniform2fvARB);
		GET_PROC_ADDRESS(Uniform2iARB);
		GET_PROC_ADDRESS(Uniform2ivARB);
		GET_PROC_ADDRESS(Uniform3fARB);
		GET_PROC_ADDRESS(Uniform3fvARB);
		GET_PROC_ADDRESS(Uniform3iARB);
		GET_PROC_ADDRESS(Uniform3ivARB);
		GET_PROC_ADDRESS(Uniform4fARB);
		GET_PROC_ADDRESS(Uniform4fvARB);
		GET_PROC_ADDRESS(Uniform4iARB);
		GET_PROC_ADDRESS(Uniform4ivARB);
		GET_PROC_ADDRESS(UniformMatrix2fvARB);
		GET_PROC_ADDRESS(UniformMatrix3fvARB);
		GET_PROC_ADDRESS(UniformMatrix4fvARB);
		GET_PROC_ADDRESS(UseProgramObjectARB);
		GET_PROC_ADDRESS(ValidateProgramARB);

	#endif // GL_ARB_shader_objects_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_compression ]----------------------------------------------------------------------------

//!	Initialize GL_ARB_texture_compression.

bool CRenderingContext::InitArbTextureCompression()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_compression_OGLEXT

		GET_PROC_ADDRESS(CompressedTexImage1DARB);
		GET_PROC_ADDRESS(CompressedTexImage2DARB);
		GET_PROC_ADDRESS(CompressedTexImage3DARB);
		GET_PROC_ADDRESS(CompressedTexSubImage1DARB);
		GET_PROC_ADDRESS(CompressedTexSubImage2DARB);
		GET_PROC_ADDRESS(CompressedTexSubImage3DARB);
		GET_PROC_ADDRESS(GetCompressedTexImageARB);

	#endif // GL_ARB_texture_compression_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_transpose_matrix ]-------------------------------------------------------------------------------

//!	Initialize GL_ARB_transpose_matrix.

bool CRenderingContext::InitArbTransposeMatrix()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_transpose_matrix_OGLEXT

		GET_PROC_ADDRESS(LoadTransposeMatrixdARB);
		GET_PROC_ADDRESS(LoadTransposeMatrixfARB);
		GET_PROC_ADDRESS(MultTransposeMatrixdARB);
		GET_PROC_ADDRESS(MultTransposeMatrixfARB);

	#endif // GL_ARB_transpose_matrix_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_vertex_blend ]-----------------------------------------------------------------------------------

//!	Initialize GL_ARB_vertex_blend.

bool CRenderingContext::InitArbVertexBlend()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_vertex_blend_OGLEXT

		GET_PROC_ADDRESS(VertexBlendARB);
		GET_PROC_ADDRESS(WeightbvARB);
		GET_PROC_ADDRESS(WeightdvARB);
		GET_PROC_ADDRESS(WeightfvARB);
		GET_PROC_ADDRESS(WeightivARB);
		GET_PROC_ADDRESS(WeightPointerARB);
		GET_PROC_ADDRESS(WeightsvARB);
		GET_PROC_ADDRESS(WeightubvARB);
		GET_PROC_ADDRESS(WeightuivARB);
		GET_PROC_ADDRESS(WeightusvARB);

	#endif // GL_ARB_vertex_blend_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_vertex_buffer_object ]---------------------------------------------------------------------------

//!	Initialize GL_ARB_vertex_buffer_object.

bool CRenderingContext::InitArbVertexBufferObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_vertex_buffer_object_OGLEXT

		GET_PROC_ADDRESS(BindBufferARB);
		GET_PROC_ADDRESS(BufferDataARB);
		GET_PROC_ADDRESS(BufferSubDataARB);
		GET_PROC_ADDRESS(DeleteBuffersARB);
		GET_PROC_ADDRESS(GenBuffersARB);
		GET_PROC_ADDRESS(GetBufferParameterivARB);
		GET_PROC_ADDRESS(GetBufferPointervARB);
		GET_PROC_ADDRESS(GetBufferSubDataARB);
		GET_PROC_ADDRESS(IsBufferARB);
		GET_PROC_ADDRESS(MapBufferARB);
		GET_PROC_ADDRESS(UnmapBufferARB);

	#endif // GL_ARB_vertex_buffer_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_vertex_program ]---------------------------------------------------------------------------------

//!	Initialize GL_ARB_vertex_program.

bool CRenderingContext::InitArbVertexProgram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_vertex_program_OGLEXT

		GET_PROC_ADDRESS(BindProgramARB);
		GET_PROC_ADDRESS(DeleteProgramsARB);
		GET_PROC_ADDRESS(DisableVertexAttribArrayARB);
		GET_PROC_ADDRESS(EnableVertexAttribArrayARB);
		GET_PROC_ADDRESS(GenProgramsARB);
		GET_PROC_ADDRESS(GetProgramEnvParameterdvARB);
		GET_PROC_ADDRESS(GetProgramEnvParameterfvARB);
		GET_PROC_ADDRESS(GetProgramivARB);
		GET_PROC_ADDRESS(GetProgramLocalParameterdvARB);
		GET_PROC_ADDRESS(GetProgramLocalParameterfvARB);
		GET_PROC_ADDRESS(GetProgramStringARB);
		GET_PROC_ADDRESS(GetVertexAttribdvARB);
		GET_PROC_ADDRESS(GetVertexAttribfvARB);
		GET_PROC_ADDRESS(GetVertexAttribivARB);
		GET_PROC_ADDRESS(GetVertexAttribPointervARB);
		GET_PROC_ADDRESS(IsProgramARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4dARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4dvARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4fARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4fvARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4dARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4dvARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4fARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4fvARB);
		GET_PROC_ADDRESS(ProgramStringARB);
		GET_PROC_ADDRESS(VertexAttrib1dARB);
		GET_PROC_ADDRESS(VertexAttrib1dvARB);
		GET_PROC_ADDRESS(VertexAttrib1fARB);
		GET_PROC_ADDRESS(VertexAttrib1fvARB);
		GET_PROC_ADDRESS(VertexAttrib1sARB);
		GET_PROC_ADDRESS(VertexAttrib1svARB);
		GET_PROC_ADDRESS(VertexAttrib2dARB);
		GET_PROC_ADDRESS(VertexAttrib2dvARB);
		GET_PROC_ADDRESS(VertexAttrib2fARB);
		GET_PROC_ADDRESS(VertexAttrib2fvARB);
		GET_PROC_ADDRESS(VertexAttrib2sARB);
		GET_PROC_ADDRESS(VertexAttrib2svARB);
		GET_PROC_ADDRESS(VertexAttrib3dARB);
		GET_PROC_ADDRESS(VertexAttrib3dvARB);
		GET_PROC_ADDRESS(VertexAttrib3fARB);
		GET_PROC_ADDRESS(VertexAttrib3fvARB);
		GET_PROC_ADDRESS(VertexAttrib3sARB);
		GET_PROC_ADDRESS(VertexAttrib3svARB);
		GET_PROC_ADDRESS(VertexAttrib4bvARB);
		GET_PROC_ADDRESS(VertexAttrib4dARB);
		GET_PROC_ADDRESS(VertexAttrib4dvARB);
		GET_PROC_ADDRESS(VertexAttrib4fARB);
		GET_PROC_ADDRESS(VertexAttrib4fvARB);
		GET_PROC_ADDRESS(VertexAttrib4ivARB);
		GET_PROC_ADDRESS(VertexAttrib4NbvARB);
		GET_PROC_ADDRESS(VertexAttrib4NivARB);
		GET_PROC_ADDRESS(VertexAttrib4NsvARB);
		GET_PROC_ADDRESS(VertexAttrib4NubARB);
		GET_PROC_ADDRESS(VertexAttrib4NubvARB);
		GET_PROC_ADDRESS(VertexAttrib4NuivARB);
		GET_PROC_ADDRESS(VertexAttrib4NusvARB);
		GET_PROC_ADDRESS(VertexAttrib4sARB);
		GET_PROC_ADDRESS(VertexAttrib4svARB);
		GET_PROC_ADDRESS(VertexAttrib4ubvARB);
		GET_PROC_ADDRESS(VertexAttrib4uivARB);
		GET_PROC_ADDRESS(VertexAttrib4usvARB);
		GET_PROC_ADDRESS(VertexAttribPointerARB);

	#endif // GL_ARB_vertex_program_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_vertex_shader ]----------------------------------------------------------------------------------

//!	Initialize GL_ARB_vertex_shader.

bool CRenderingContext::InitArbVertexShader()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_vertex_shader_OGLEXT

		GET_PROC_ADDRESS(BindAttribLocationARB);
		GET_PROC_ADDRESS(GetActiveAttribARB);
		GET_PROC_ADDRESS(GetAttribLocationARB);

	#endif // GL_ARB_vertex_shader_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_window_pos ]-------------------------------------------------------------------------------------

//!	Initialize GL_ARB_window_pos.

bool CRenderingContext::InitArbWindowPos()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_window_pos_OGLEXT

		GET_PROC_ADDRESS(WindowPos2dARB);
		GET_PROC_ADDRESS(WindowPos2dvARB);
		GET_PROC_ADDRESS(WindowPos2fARB);
		GET_PROC_ADDRESS(WindowPos2fvARB);
		GET_PROC_ADDRESS(WindowPos2iARB);
		GET_PROC_ADDRESS(WindowPos2ivARB);
		GET_PROC_ADDRESS(WindowPos2sARB);
		GET_PROC_ADDRESS(WindowPos2svARB);
		GET_PROC_ADDRESS(WindowPos3dARB);
		GET_PROC_ADDRESS(WindowPos3dvARB);
		GET_PROC_ADDRESS(WindowPos3fARB);
		GET_PROC_ADDRESS(WindowPos3fvARB);
		GET_PROC_ADDRESS(WindowPos3iARB);
		GET_PROC_ADDRESS(WindowPos3ivARB);
		GET_PROC_ADDRESS(WindowPos3sARB);
		GET_PROC_ADDRESS(WindowPos3svARB);

	#endif // GL_ARB_window_pos_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_draw_buffers ]-----------------------------------------------------------------------------------

//!	Initialize GL_ATI_draw_buffers.

bool CRenderingContext::InitAtiDrawBuffers()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_draw_buffers_OGLEXT

		GET_PROC_ADDRESS(DrawBuffersATI);

	#endif // GL_ATI_draw_buffers_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_element_array ]----------------------------------------------------------------------------------

//!	Initialize GL_ATI_element_array.

bool CRenderingContext::InitAtiElementArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_element_array_OGLEXT

		GET_PROC_ADDRESS(DrawElementArrayATI);
		GET_PROC_ADDRESS(DrawRangeElementArrayATI);
		GET_PROC_ADDRESS(ElementPointerATI);

	#endif // GL_ATI_element_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_envmap_bumpmap ]---------------------------------------------------------------------------------

//!	Initialize GL_ATI_envmap_bumpmap.

bool CRenderingContext::InitAtiEnvmapBumpmap()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_envmap_bumpmap_OGLEXT

		GET_PROC_ADDRESS(GetTexBumpParameterfvATI);
		GET_PROC_ADDRESS(GetTexBumpParameterivATI);
		GET_PROC_ADDRESS(TexBumpParameterfvATI);
		GET_PROC_ADDRESS(TexBumpParameterivATI);

	#endif // GL_ATI_envmap_bumpmap_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_fragment_shader ]--------------------------------------------------------------------------------

//!	Initialize GL_ATI_fragment_shader.

bool CRenderingContext::InitAtiFragmentShader()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_fragment_shader_OGLEXT

		GET_PROC_ADDRESS(AlphaFragmentOp1ATI);
		GET_PROC_ADDRESS(AlphaFragmentOp2ATI);
		GET_PROC_ADDRESS(AlphaFragmentOp3ATI);
		GET_PROC_ADDRESS(BeginFragmentShaderATI);
		GET_PROC_ADDRESS(BindFragmentShaderATI);
		GET_PROC_ADDRESS(ColorFragmentOp1ATI);
		GET_PROC_ADDRESS(ColorFragmentOp2ATI);
		GET_PROC_ADDRESS(ColorFragmentOp3ATI);
		GET_PROC_ADDRESS(DeleteFragmentShaderATI);
		GET_PROC_ADDRESS(EndFragmentShaderATI);
		GET_PROC_ADDRESS(GenFragmentShadersATI);
		GET_PROC_ADDRESS(PassTexCoordATI);
		GET_PROC_ADDRESS(SampleMapATI);
		GET_PROC_ADDRESS(SetFragmentShaderConstantATI);

	#endif // GL_ATI_fragment_shader_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_map_object_buffer ]------------------------------------------------------------------------------

//!	Initialize GL_ATI_map_object_buffer.

bool CRenderingContext::InitAtiMapObjectBuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_map_object_buffer_OGLEXT

		GET_PROC_ADDRESS(MapObjectBufferATI);
		GET_PROC_ADDRESS(UnmapObjectBufferATI);

	#endif // GL_ATI_map_object_buffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_pn_triangles ]-----------------------------------------------------------------------------------

//!	Initialize GL_ATI_pn_triangles.

bool CRenderingContext::InitAtiPnTriangles()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_pn_triangles_OGLEXT

		GET_PROC_ADDRESS(PNTrianglesfATI);
		GET_PROC_ADDRESS(PNTrianglesiATI);

	#endif // GL_ATI_pn_triangles_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_separate_stencil ]-------------------------------------------------------------------------------

//!	Initialize GL_ATI_separate_stencil.

bool CRenderingContext::InitAtiSeparateStencil()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_separate_stencil_OGLEXT

		GET_PROC_ADDRESS(StencilFuncSeparateATI);
		GET_PROC_ADDRESS(StencilOpSeparateATI);

	#endif // GL_ATI_separate_stencil_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_vertex_array_object ]----------------------------------------------------------------------------

//!	Initialize GL_ATI_vertex_array_object.

bool CRenderingContext::InitAtiVertexArrayObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_vertex_array_object_OGLEXT

		GET_PROC_ADDRESS(ArrayObjectATI);
		GET_PROC_ADDRESS(FreeObjectBufferATI);
		GET_PROC_ADDRESS(GetArrayObjectfvATI);
		GET_PROC_ADDRESS(GetArrayObjectivATI);
		GET_PROC_ADDRESS(GetObjectBufferfvATI);
		GET_PROC_ADDRESS(GetObjectBufferivATI);
		GET_PROC_ADDRESS(GetVariantArrayObjectfvATI);
		GET_PROC_ADDRESS(GetVariantArrayObjectivATI);
		GET_PROC_ADDRESS(IsObjectBufferATI);
		GET_PROC_ADDRESS(NewObjectBufferATI);
		GET_PROC_ADDRESS(UpdateObjectBufferATI);
		GET_PROC_ADDRESS(VariantArrayObjectATI);

	#endif // GL_ATI_vertex_array_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_vertex_attrib_array_object ]---------------------------------------------------------------------

//!	Initialize GL_ATI_vertex_attrib_array_object.

bool CRenderingContext::InitAtiVertexAttribArrayObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_vertex_attrib_array_object_OGLEXT

		GET_PROC_ADDRESS(GetVertexAttribArrayObjectfvATI);
		GET_PROC_ADDRESS(GetVertexAttribArrayObjectivATI);
		GET_PROC_ADDRESS(VertexAttribArrayObjectATI);

	#endif // GL_ATI_vertex_attrib_array_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_vertex_streams ]---------------------------------------------------------------------------------

//!	Initialize GL_ATI_vertex_streams.

bool CRenderingContext::InitAtiVertexStreams()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_vertex_streams_OGLEXT

		GET_PROC_ADDRESS(ClientActiveVertexStreamATI);
		GET_PROC_ADDRESS(NormalStream3bATI);
		GET_PROC_ADDRESS(NormalStream3bvATI);
		GET_PROC_ADDRESS(NormalStream3dATI);
		GET_PROC_ADDRESS(NormalStream3dvATI);
		GET_PROC_ADDRESS(NormalStream3fATI);
		GET_PROC_ADDRESS(NormalStream3fvATI);
		GET_PROC_ADDRESS(NormalStream3iATI);
		GET_PROC_ADDRESS(NormalStream3ivATI);
		GET_PROC_ADDRESS(NormalStream3sATI);
		GET_PROC_ADDRESS(NormalStream3svATI);
		GET_PROC_ADDRESS(VertexBlendEnvfATI);
		GET_PROC_ADDRESS(VertexBlendEnviATI);
		GET_PROC_ADDRESS(VertexStream1dATI);
		GET_PROC_ADDRESS(VertexStream1dvATI);
		GET_PROC_ADDRESS(VertexStream1fATI);
		GET_PROC_ADDRESS(VertexStream1fvATI);
		GET_PROC_ADDRESS(VertexStream1iATI);
		GET_PROC_ADDRESS(VertexStream1ivATI);
		GET_PROC_ADDRESS(VertexStream1sATI);
		GET_PROC_ADDRESS(VertexStream1svATI);
		GET_PROC_ADDRESS(VertexStream2dATI);
		GET_PROC_ADDRESS(VertexStream2dvATI);
		GET_PROC_ADDRESS(VertexStream2fATI);
		GET_PROC_ADDRESS(VertexStream2fvATI);
		GET_PROC_ADDRESS(VertexStream2iATI);
		GET_PROC_ADDRESS(VertexStream2ivATI);
		GET_PROC_ADDRESS(VertexStream2sATI);
		GET_PROC_ADDRESS(VertexStream2svATI);
		GET_PROC_ADDRESS(VertexStream3dATI);
		GET_PROC_ADDRESS(VertexStream3dvATI);
		GET_PROC_ADDRESS(VertexStream3fATI);
		GET_PROC_ADDRESS(VertexStream3fvATI);
		GET_PROC_ADDRESS(VertexStream3iATI);
		GET_PROC_ADDRESS(VertexStream3ivATI);
		GET_PROC_ADDRESS(VertexStream3sATI);
		GET_PROC_ADDRESS(VertexStream3svATI);
		GET_PROC_ADDRESS(VertexStream4dATI);
		GET_PROC_ADDRESS(VertexStream4dvATI);
		GET_PROC_ADDRESS(VertexStream4fATI);
		GET_PROC_ADDRESS(VertexStream4fvATI);
		GET_PROC_ADDRESS(VertexStream4iATI);
		GET_PROC_ADDRESS(VertexStream4ivATI);
		GET_PROC_ADDRESS(VertexStream4sATI);
		GET_PROC_ADDRESS(VertexStream4svATI);

	#endif // GL_ATI_vertex_streams_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_color ]------------------------------------------------------------------------------------

//!	Initialize GL_EXT_blend_color.

bool CRenderingContext::InitExtBlendColor()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_color_OGLEXT

		GET_PROC_ADDRESS(BlendColorEXT);

	#endif // GL_EXT_blend_color_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_equation_separate ]------------------------------------------------------------------------

//!	Initialize GL_EXT_blend_equation_separate.

bool CRenderingContext::InitExtBlendEquationSeparate()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_equation_separate_OGLEXT

		GET_PROC_ADDRESS(BlendEquationSeparateEXT);

	#endif // GL_EXT_blend_equation_separate_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_func_separate ]----------------------------------------------------------------------------

//!	Initialize GL_EXT_blend_func_separate.

bool CRenderingContext::InitExtBlendFuncSeparate()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_func_separate_OGLEXT

		GET_PROC_ADDRESS(BlendFuncSeparateEXT);

	#endif // GL_EXT_blend_func_separate_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_minmax ]-----------------------------------------------------------------------------------

//!	Initialize GL_EXT_blend_minmax.

bool CRenderingContext::InitExtBlendMinmax()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_minmax_OGLEXT

		GET_PROC_ADDRESS(BlendEquationEXT);

	#endif // GL_EXT_blend_minmax_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_color_subtable ]---------------------------------------------------------------------------------

//!	Initialize GL_EXT_color_subtable.

bool CRenderingContext::InitExtColorSubtable()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_color_subtable_OGLEXT

#ifndef WIN32 // kind of hack, functions are never used in DftD
		GET_PROC_ADDRESS(ColorSubTableEXT);
		GET_PROC_ADDRESS(CopyColorSubTableEXT);
#endif

	#endif // GL_EXT_color_subtable_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_compiled_vertex_array ]--------------------------------------------------------------------------

//!	Initialize GL_EXT_compiled_vertex_array.

bool CRenderingContext::InitExtCompiledVertexArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_compiled_vertex_array_OGLEXT

		GET_PROC_ADDRESS(LockArraysEXT);
		GET_PROC_ADDRESS(UnlockArraysEXT);

	#endif // GL_EXT_compiled_vertex_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_convolution ]------------------------------------------------------------------------------------

//!	Initialize GL_EXT_convolution.

bool CRenderingContext::InitExtConvolution()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_convolution_OGLEXT

		GET_PROC_ADDRESS(ConvolutionFilter1DEXT);
		GET_PROC_ADDRESS(ConvolutionFilter2DEXT);
		GET_PROC_ADDRESS(ConvolutionParameterfEXT);
		GET_PROC_ADDRESS(ConvolutionParameterfvEXT);
		GET_PROC_ADDRESS(ConvolutionParameteriEXT);
		GET_PROC_ADDRESS(ConvolutionParameterivEXT);
		GET_PROC_ADDRESS(CopyConvolutionFilter1DEXT);
		GET_PROC_ADDRESS(CopyConvolutionFilter2DEXT);
		GET_PROC_ADDRESS(GetConvolutionFilterEXT);
		GET_PROC_ADDRESS(GetConvolutionParameterfvEXT);
		GET_PROC_ADDRESS(GetConvolutionParameterivEXT);
		GET_PROC_ADDRESS(GetSeparableFilterEXT);
		GET_PROC_ADDRESS(SeparableFilter2DEXT);

	#endif // GL_EXT_convolution_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_coordinate_frame ]-------------------------------------------------------------------------------

//!	Initialize GL_EXT_coordinate_frame.

bool CRenderingContext::InitExtCoordinateFrame()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_coordinate_frame_OGLEXT

		GET_PROC_ADDRESS(Binormal3bEXT);
		GET_PROC_ADDRESS(Binormal3bvEXT);
		GET_PROC_ADDRESS(Binormal3dEXT);
		GET_PROC_ADDRESS(Binormal3dvEXT);
		GET_PROC_ADDRESS(Binormal3fEXT);
		GET_PROC_ADDRESS(Binormal3fvEXT);
		GET_PROC_ADDRESS(Binormal3iEXT);
		GET_PROC_ADDRESS(Binormal3ivEXT);
		GET_PROC_ADDRESS(Binormal3sEXT);
		GET_PROC_ADDRESS(Binormal3svEXT);
		GET_PROC_ADDRESS(BinormalPointerEXT);
		GET_PROC_ADDRESS(Tangent3bEXT);
		GET_PROC_ADDRESS(Tangent3bvEXT);
		GET_PROC_ADDRESS(Tangent3dEXT);
		GET_PROC_ADDRESS(Tangent3dvEXT);
		GET_PROC_ADDRESS(Tangent3fEXT);
		GET_PROC_ADDRESS(Tangent3fvEXT);
		GET_PROC_ADDRESS(Tangent3iEXT);
		GET_PROC_ADDRESS(Tangent3ivEXT);
		GET_PROC_ADDRESS(Tangent3sEXT);
		GET_PROC_ADDRESS(Tangent3svEXT);
		GET_PROC_ADDRESS(TangentPointerEXT);

	#endif // GL_EXT_coordinate_frame_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_copy_texture ]-----------------------------------------------------------------------------------

//!	Initialize GL_EXT_copy_texture.

bool CRenderingContext::InitExtCopyTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_copy_texture_OGLEXT

		GET_PROC_ADDRESS(CopyTexImage1DEXT);
		GET_PROC_ADDRESS(CopyTexImage2DEXT);
		GET_PROC_ADDRESS(CopyTexSubImage1DEXT);
		GET_PROC_ADDRESS(CopyTexSubImage2DEXT);
		GET_PROC_ADDRESS(CopyTexSubImage3DEXT);

	#endif // GL_EXT_copy_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_cull_vertex ]------------------------------------------------------------------------------------

//!	Initialize GL_EXT_cull_vertex.

bool CRenderingContext::InitExtCullVertex()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_cull_vertex_OGLEXT

		GET_PROC_ADDRESS(CullParameterdvEXT);
		GET_PROC_ADDRESS(CullParameterfvEXT);

	#endif // GL_EXT_cull_vertex_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_depth_bounds_test ]------------------------------------------------------------------------------

//!	Initialize GL_EXT_depth_bounds_test.

bool CRenderingContext::InitExtDepthBoundsTest()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_depth_bounds_test_OGLEXT

		GET_PROC_ADDRESS(DepthBoundsEXT);

	#endif // GL_EXT_depth_bounds_test_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_draw_range_elements ]----------------------------------------------------------------------------

//!	Initialize GL_EXT_draw_range_elements.

bool CRenderingContext::InitExtDrawRangeElements()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_draw_range_elements_OGLEXT

		GET_PROC_ADDRESS(DrawRangeElementsEXT);

	#endif // GL_EXT_draw_range_elements_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_fog_coord ]--------------------------------------------------------------------------------------

//!	Initialize GL_EXT_fog_coord.

bool CRenderingContext::InitExtFogCoord()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_fog_coord_OGLEXT

		GET_PROC_ADDRESS(FogCoorddEXT);
		GET_PROC_ADDRESS(FogCoorddvEXT);
		GET_PROC_ADDRESS(FogCoordfEXT);
		GET_PROC_ADDRESS(FogCoordfvEXT);
		GET_PROC_ADDRESS(FogCoordPointerEXT);

	#endif // GL_EXT_fog_coord_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_framebuffer_object ]-----------------------------------------------------------------------------

//!	Initialize GL_EXT_framebuffer_object.

bool CRenderingContext::InitExtFramebufferObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_framebuffer_object_OGLEXT

		GET_PROC_ADDRESS(BindFramebufferEXT);
		GET_PROC_ADDRESS(BindRenderbufferEXT);
		GET_PROC_ADDRESS(CheckFramebufferStatusEXT);
		GET_PROC_ADDRESS(DeleteFramebuffersEXT);
		GET_PROC_ADDRESS(DeleteRenderbuffersEXT);
		GET_PROC_ADDRESS(FramebufferRenderbufferEXT);
		GET_PROC_ADDRESS(FramebufferTexture1DEXT);
		GET_PROC_ADDRESS(FramebufferTexture2DEXT);
		GET_PROC_ADDRESS(FramebufferTexture3DEXT);
		GET_PROC_ADDRESS(GenerateMipmapEXT);
		GET_PROC_ADDRESS(GenFramebuffersEXT);
		GET_PROC_ADDRESS(GenRenderbuffersEXT);
		GET_PROC_ADDRESS(GetFramebufferAttachmentParameterivEXT);
		GET_PROC_ADDRESS(GetRenderbufferParameterivEXT);
		GET_PROC_ADDRESS(IsFramebufferEXT);
		GET_PROC_ADDRESS(IsRenderbufferEXT);
		GET_PROC_ADDRESS(RenderbufferStorageEXT);

	#endif // GL_EXT_framebuffer_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_histogram ]--------------------------------------------------------------------------------------

//!	Initialize GL_EXT_histogram.

bool CRenderingContext::InitExtHistogram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_histogram_OGLEXT

		GET_PROC_ADDRESS(GetHistogramEXT);
		GET_PROC_ADDRESS(GetHistogramParameterfvEXT);
		GET_PROC_ADDRESS(GetHistogramParameterivEXT);
		GET_PROC_ADDRESS(GetMinmaxEXT);
		GET_PROC_ADDRESS(GetMinmaxParameterfvEXT);
		GET_PROC_ADDRESS(GetMinmaxParameterivEXT);
		GET_PROC_ADDRESS(HistogramEXT);
		GET_PROC_ADDRESS(MinmaxEXT);
		GET_PROC_ADDRESS(ResetHistogramEXT);
		GET_PROC_ADDRESS(ResetMinmaxEXT);

	#endif // GL_EXT_histogram_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_index_func ]-------------------------------------------------------------------------------------

//!	Initialize GL_EXT_index_func.

bool CRenderingContext::InitExtIndexFunc()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_index_func_OGLEXT

		GET_PROC_ADDRESS(IndexFuncEXT);

	#endif // GL_EXT_index_func_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_index_material ]---------------------------------------------------------------------------------

//!	Initialize GL_EXT_index_material.

bool CRenderingContext::InitExtIndexMaterial()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_index_material_OGLEXT

		GET_PROC_ADDRESS(IndexMaterialEXT);

	#endif // GL_EXT_index_material_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_light_texture ]----------------------------------------------------------------------------------

//!	Initialize GL_EXT_light_texture.

bool CRenderingContext::InitExtLightTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_light_texture_OGLEXT

		GET_PROC_ADDRESS(ApplyTextureEXT);
		GET_PROC_ADDRESS(TextureLightEXT);
		GET_PROC_ADDRESS(TextureMaterialEXT);

	#endif // GL_EXT_light_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_multi_draw_arrays ]------------------------------------------------------------------------------

//!	Initialize GL_EXT_multi_draw_arrays.

bool CRenderingContext::InitExtMultiDrawArrays()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_multi_draw_arrays_OGLEXT

		GET_PROC_ADDRESS(MultiDrawArraysEXT);
		GET_PROC_ADDRESS(MultiDrawElementsEXT);

	#endif // GL_EXT_multi_draw_arrays_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_multisample ]------------------------------------------------------------------------------------

//!	Initialize GL_EXT_multisample.

bool CRenderingContext::InitExtMultisample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_multisample_OGLEXT

		GET_PROC_ADDRESS(SampleMaskEXT);
		GET_PROC_ADDRESS(SamplePatternEXT);

	#endif // GL_EXT_multisample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_paletted_texture ]-------------------------------------------------------------------------------

//!	Initialize GL_EXT_paletted_texture.

bool CRenderingContext::InitExtPalettedTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_paletted_texture_OGLEXT

		GET_PROC_ADDRESS(ColorSubTableEXT);
		GET_PROC_ADDRESS(ColorTableEXT);
		GET_PROC_ADDRESS(GetColorTableEXT);
		GET_PROC_ADDRESS(GetColorTableParameterfvEXT);
		GET_PROC_ADDRESS(GetColorTableParameterivEXT);

	#endif // GL_EXT_paletted_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_pixel_transform ]--------------------------------------------------------------------------------

//!	Initialize GL_EXT_pixel_transform.

bool CRenderingContext::InitExtPixelTransform()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_pixel_transform_OGLEXT

		GET_PROC_ADDRESS(PixelTransformParameterfEXT);
		GET_PROC_ADDRESS(PixelTransformParameterfvEXT);
		GET_PROC_ADDRESS(PixelTransformParameteriEXT);
		GET_PROC_ADDRESS(PixelTransformParameterivEXT);

	#endif // GL_EXT_pixel_transform_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_point_parameters ]-------------------------------------------------------------------------------

//!	Initialize GL_EXT_point_parameters.

bool CRenderingContext::InitExtPointParameters()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_point_parameters_OGLEXT

		GET_PROC_ADDRESS(PointParameterfEXT);
		GET_PROC_ADDRESS(PointParameterfvEXT);

	#endif // GL_EXT_point_parameters_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_polygon_offset ]---------------------------------------------------------------------------------

//!	Initialize GL_EXT_polygon_offset.

bool CRenderingContext::InitExtPolygonOffset()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_polygon_offset_OGLEXT

		GET_PROC_ADDRESS(PolygonOffsetEXT);

	#endif // GL_EXT_polygon_offset_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_secondary_color ]--------------------------------------------------------------------------------

//!	Initialize GL_EXT_secondary_color.

bool CRenderingContext::InitExtSecondaryColor()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_secondary_color_OGLEXT

		GET_PROC_ADDRESS(SecondaryColor3bEXT);
		GET_PROC_ADDRESS(SecondaryColor3bvEXT);
		GET_PROC_ADDRESS(SecondaryColor3dEXT);
		GET_PROC_ADDRESS(SecondaryColor3dvEXT);
		GET_PROC_ADDRESS(SecondaryColor3fEXT);
		GET_PROC_ADDRESS(SecondaryColor3fvEXT);
		GET_PROC_ADDRESS(SecondaryColor3iEXT);
		GET_PROC_ADDRESS(SecondaryColor3ivEXT);
		GET_PROC_ADDRESS(SecondaryColor3sEXT);
		GET_PROC_ADDRESS(SecondaryColor3svEXT);
		GET_PROC_ADDRESS(SecondaryColor3ubEXT);
		GET_PROC_ADDRESS(SecondaryColor3ubvEXT);
		GET_PROC_ADDRESS(SecondaryColor3uiEXT);
		GET_PROC_ADDRESS(SecondaryColor3uivEXT);
		GET_PROC_ADDRESS(SecondaryColor3usEXT);
		GET_PROC_ADDRESS(SecondaryColor3usvEXT);
		GET_PROC_ADDRESS(SecondaryColorPointerEXT);

	#endif // GL_EXT_secondary_color_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_stencil_two_side ]-------------------------------------------------------------------------------

//!	Initialize GL_EXT_stencil_two_side.

bool CRenderingContext::InitExtStencilTwoSide()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_stencil_two_side_OGLEXT

		GET_PROC_ADDRESS(ActiveStencilFaceEXT);

	#endif // GL_EXT_stencil_two_side_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_subtexture ]-------------------------------------------------------------------------------------

//!	Initialize GL_EXT_subtexture.

bool CRenderingContext::InitExtSubtexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_subtexture_OGLEXT

		GET_PROC_ADDRESS(TexSubImage1DEXT);
		GET_PROC_ADDRESS(TexSubImage2DEXT);

	#endif // GL_EXT_subtexture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture3D ]--------------------------------------------------------------------------------------

//!	Initialize GL_EXT_texture3D.

bool CRenderingContext::InitExtTexture3d()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture3D_OGLEXT

		GET_PROC_ADDRESS(TexImage3DEXT);
		GET_PROC_ADDRESS(TexSubImage3DEXT);

	#endif // GL_EXT_texture3D_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_object ]---------------------------------------------------------------------------------

//!	Initialize GL_EXT_texture_object.

bool CRenderingContext::InitExtTextureObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_object_OGLEXT

		GET_PROC_ADDRESS(AreTexturesResidentEXT);
		GET_PROC_ADDRESS(BindTextureEXT);
		GET_PROC_ADDRESS(DeleteTexturesEXT);
		GET_PROC_ADDRESS(GenTexturesEXT);
		GET_PROC_ADDRESS(IsTextureEXT);
		GET_PROC_ADDRESS(PrioritizeTexturesEXT);

	#endif // GL_EXT_texture_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_perturb_normal ]-------------------------------------------------------------------------

//!	Initialize GL_EXT_texture_perturb_normal.

bool CRenderingContext::InitExtTexturePerturbNormal()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_perturb_normal_OGLEXT

		GET_PROC_ADDRESS(TextureNormalEXT);

	#endif // GL_EXT_texture_perturb_normal_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_vertex_array ]-----------------------------------------------------------------------------------

//!	Initialize GL_EXT_vertex_array.

bool CRenderingContext::InitExtVertexArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_vertex_array_OGLEXT

		GET_PROC_ADDRESS(ArrayElementEXT);
		GET_PROC_ADDRESS(ColorPointerEXT);
		GET_PROC_ADDRESS(DrawArraysEXT);
		GET_PROC_ADDRESS(EdgeFlagPointerEXT);
		GET_PROC_ADDRESS(GetPointervEXT);
		GET_PROC_ADDRESS(IndexPointerEXT);
		GET_PROC_ADDRESS(NormalPointerEXT);
		GET_PROC_ADDRESS(TexCoordPointerEXT);
		GET_PROC_ADDRESS(VertexPointerEXT);

	#endif // GL_EXT_vertex_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_vertex_shader ]----------------------------------------------------------------------------------

//!	Initialize GL_EXT_vertex_shader.

bool CRenderingContext::InitExtVertexShader()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_vertex_shader_OGLEXT

		GET_PROC_ADDRESS(BeginVertexShaderEXT);
		GET_PROC_ADDRESS(BindLightParameterEXT);
		GET_PROC_ADDRESS(BindMaterialParameterEXT);
		GET_PROC_ADDRESS(BindParameterEXT);
		GET_PROC_ADDRESS(BindTexGenParameterEXT);
		GET_PROC_ADDRESS(BindTextureUnitParameterEXT);
		GET_PROC_ADDRESS(BindVertexShaderEXT);
		GET_PROC_ADDRESS(DeleteVertexShaderEXT);
		GET_PROC_ADDRESS(DisableVariantClientStateEXT);
		GET_PROC_ADDRESS(EnableVariantClientStateEXT);
		GET_PROC_ADDRESS(EndVertexShaderEXT);
		GET_PROC_ADDRESS(ExtractComponentEXT);
		GET_PROC_ADDRESS(GenSymbolsEXT);
		GET_PROC_ADDRESS(GenVertexShadersEXT);
		GET_PROC_ADDRESS(GetInvariantBooleanvEXT);
		GET_PROC_ADDRESS(GetInvariantFloatvEXT);
		GET_PROC_ADDRESS(GetInvariantIntegervEXT);
		GET_PROC_ADDRESS(GetLocalConstantBooleanvEXT);
		GET_PROC_ADDRESS(GetLocalConstantFloatvEXT);
		GET_PROC_ADDRESS(GetLocalConstantIntegervEXT);
		GET_PROC_ADDRESS(GetVariantBooleanvEXT);
		GET_PROC_ADDRESS(GetVariantFloatvEXT);
		GET_PROC_ADDRESS(GetVariantIntegervEXT);
		GET_PROC_ADDRESS(GetVariantPointervEXT);
		GET_PROC_ADDRESS(InsertComponentEXT);
		GET_PROC_ADDRESS(IsVariantEnabledEXT);
		GET_PROC_ADDRESS(SetInvariantEXT);
		GET_PROC_ADDRESS(SetLocalConstantEXT);
		GET_PROC_ADDRESS(ShaderOp1EXT);
		GET_PROC_ADDRESS(ShaderOp2EXT);
		GET_PROC_ADDRESS(ShaderOp3EXT);
		GET_PROC_ADDRESS(SwizzleEXT);
		GET_PROC_ADDRESS(VariantbvEXT);
		GET_PROC_ADDRESS(VariantdvEXT);
		GET_PROC_ADDRESS(VariantfvEXT);
		GET_PROC_ADDRESS(VariantivEXT);
		GET_PROC_ADDRESS(VariantPointerEXT);
		GET_PROC_ADDRESS(VariantsvEXT);
		GET_PROC_ADDRESS(VariantubvEXT);
		GET_PROC_ADDRESS(VariantuivEXT);
		GET_PROC_ADDRESS(VariantusvEXT);
		GET_PROC_ADDRESS(WriteMaskEXT);

	#endif // GL_EXT_vertex_shader_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_vertex_weighting ]-------------------------------------------------------------------------------

//!	Initialize GL_EXT_vertex_weighting.

bool CRenderingContext::InitExtVertexWeighting()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_vertex_weighting_OGLEXT

		GET_PROC_ADDRESS(VertexWeightfEXT);
		GET_PROC_ADDRESS(VertexWeightfvEXT);
		GET_PROC_ADDRESS(VertexWeightPointerEXT);

	#endif // GL_EXT_vertex_weighting_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_GREMEDY_string_marker ]------------------------------------------------------------------------------

//!	Initialize GL_GREMEDY_string_marker.

bool CRenderingContext::InitGremedyStringMarker()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_GREMEDY_string_marker_OGLEXT

		GET_PROC_ADDRESS(StringMarkerGREMEDY);

	#endif // GL_GREMEDY_string_marker_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_HP_image_transform ]---------------------------------------------------------------------------------

//!	Initialize GL_HP_image_transform.

bool CRenderingContext::InitHpImageTransform()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_HP_image_transform_OGLEXT

		GET_PROC_ADDRESS(GetImageTransformParameterfvHP);
		GET_PROC_ADDRESS(GetImageTransformParameterivHP);
		GET_PROC_ADDRESS(ImageTransformParameterfHP);
		GET_PROC_ADDRESS(ImageTransformParameterfvHP);
		GET_PROC_ADDRESS(ImageTransformParameteriHP);
		GET_PROC_ADDRESS(ImageTransformParameterivHP);

	#endif // GL_HP_image_transform_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_multimode_draw_arrays ]--------------------------------------------------------------------------

//!	Initialize GL_IBM_multimode_draw_arrays.

bool CRenderingContext::InitIbmMultimodeDrawArrays()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_multimode_draw_arrays_OGLEXT

		GET_PROC_ADDRESS(MultiModeDrawArraysIBM);
		GET_PROC_ADDRESS(MultiModeDrawElementsIBM);

	#endif // GL_IBM_multimode_draw_arrays_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_vertex_array_lists ]-----------------------------------------------------------------------------

//!	Initialize GL_IBM_vertex_array_lists.

bool CRenderingContext::InitIbmVertexArrayLists()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_vertex_array_lists_OGLEXT

		GET_PROC_ADDRESS(ColorPointerListIBM);
		GET_PROC_ADDRESS(EdgeFlagPointerListIBM);
		GET_PROC_ADDRESS(FogCoordPointerListIBM);
		GET_PROC_ADDRESS(IndexPointerListIBM);
		GET_PROC_ADDRESS(NormalPointerListIBM);
		GET_PROC_ADDRESS(SecondaryColorPointerListIBM);
		GET_PROC_ADDRESS(TexCoordPointerListIBM);
		GET_PROC_ADDRESS(VertexPointerListIBM);

	#endif // GL_IBM_vertex_array_lists_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_INGR_blend_func_separate ]---------------------------------------------------------------------------

//!	Initialize GL_INGR_blend_func_separate.

bool CRenderingContext::InitIngrBlendFuncSeparate()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_INGR_blend_func_separate_OGLEXT

		GET_PROC_ADDRESS(BlendFuncSeparateINGR);

	#endif // GL_INGR_blend_func_separate_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_INTEL_parallel_arrays ]------------------------------------------------------------------------------

//!	Initialize GL_INTEL_parallel_arrays.

bool CRenderingContext::InitIntelParallelArrays()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_INTEL_parallel_arrays_OGLEXT

		GET_PROC_ADDRESS(ColorPointervINTEL);
		GET_PROC_ADDRESS(NormalPointervINTEL);
		GET_PROC_ADDRESS(TexCoordPointervINTEL);
		GET_PROC_ADDRESS(VertexPointervINTEL);

	#endif // GL_INTEL_parallel_arrays_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_MESA_resize_buffers ]--------------------------------------------------------------------------------

//!	Initialize GL_MESA_resize_buffers.

bool CRenderingContext::InitMesaResizeBuffers()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_MESA_resize_buffers_OGLEXT

		GET_PROC_ADDRESS(ResizeBuffersMESA);

	#endif // GL_MESA_resize_buffers_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_MESA_window_pos ]------------------------------------------------------------------------------------

//!	Initialize GL_MESA_window_pos.

bool CRenderingContext::InitMesaWindowPos()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_MESA_window_pos_OGLEXT

		GET_PROC_ADDRESS(WindowPos2dMESA);
		GET_PROC_ADDRESS(WindowPos2dvMESA);
		GET_PROC_ADDRESS(WindowPos2fMESA);
		GET_PROC_ADDRESS(WindowPos2fvMESA);
		GET_PROC_ADDRESS(WindowPos2iMESA);
		GET_PROC_ADDRESS(WindowPos2ivMESA);
		GET_PROC_ADDRESS(WindowPos2sMESA);
		GET_PROC_ADDRESS(WindowPos2svMESA);
		GET_PROC_ADDRESS(WindowPos3dMESA);
		GET_PROC_ADDRESS(WindowPos3dvMESA);
		GET_PROC_ADDRESS(WindowPos3fMESA);
		GET_PROC_ADDRESS(WindowPos3fvMESA);
		GET_PROC_ADDRESS(WindowPos3iMESA);
		GET_PROC_ADDRESS(WindowPos3ivMESA);
		GET_PROC_ADDRESS(WindowPos3sMESA);
		GET_PROC_ADDRESS(WindowPos3svMESA);
		GET_PROC_ADDRESS(WindowPos4dMESA);
		GET_PROC_ADDRESS(WindowPos4dvMESA);
		GET_PROC_ADDRESS(WindowPos4fMESA);
		GET_PROC_ADDRESS(WindowPos4fvMESA);
		GET_PROC_ADDRESS(WindowPos4iMESA);
		GET_PROC_ADDRESS(WindowPos4ivMESA);
		GET_PROC_ADDRESS(WindowPos4sMESA);
		GET_PROC_ADDRESS(WindowPos4svMESA);

	#endif // GL_MESA_window_pos_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_element_array ]-----------------------------------------------------------------------------------

//!	Initialize GL_NV_element_array.

bool CRenderingContext::InitNvElementArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_element_array_OGLEXT

		GET_PROC_ADDRESS(DrawElementArrayNV);
		GET_PROC_ADDRESS(DrawRangeElementArrayNV);
		GET_PROC_ADDRESS(ElementPointerNV);
		GET_PROC_ADDRESS(MultiDrawElementArrayNV);
		GET_PROC_ADDRESS(MultiDrawRangeElementArrayNV);

	#endif // GL_NV_element_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_evaluators ]--------------------------------------------------------------------------------------

//!	Initialize GL_NV_evaluators.

bool CRenderingContext::InitNvEvaluators()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_evaluators_OGLEXT

		GET_PROC_ADDRESS(EvalMapsNV);
		GET_PROC_ADDRESS(GetMapAttribParameterfvNV);
		GET_PROC_ADDRESS(GetMapAttribParameterivNV);
		GET_PROC_ADDRESS(GetMapControlPointsNV);
		GET_PROC_ADDRESS(GetMapParameterfvNV);
		GET_PROC_ADDRESS(GetMapParameterivNV);
		GET_PROC_ADDRESS(MapControlPointsNV);
		GET_PROC_ADDRESS(MapParameterfvNV);
		GET_PROC_ADDRESS(MapParameterivNV);

	#endif // GL_NV_evaluators_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_fence ]-------------------------------------------------------------------------------------------

//!	Initialize GL_NV_fence.

bool CRenderingContext::InitNvFence()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_fence_OGLEXT

		GET_PROC_ADDRESS(DeleteFencesNV);
		GET_PROC_ADDRESS(FinishFenceNV);
		GET_PROC_ADDRESS(GenFencesNV);
		GET_PROC_ADDRESS(GetFenceivNV);
		GET_PROC_ADDRESS(IsFenceNV);
		GET_PROC_ADDRESS(SetFenceNV);
		GET_PROC_ADDRESS(TestFenceNV);

	#endif // GL_NV_fence_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_fragment_program ]--------------------------------------------------------------------------------

//!	Initialize GL_NV_fragment_program.

bool CRenderingContext::InitNvFragmentProgram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_fragment_program_OGLEXT

		GET_PROC_ADDRESS(GetProgramNamedParameterdvNV);
		GET_PROC_ADDRESS(GetProgramNamedParameterfvNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4dNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4dvNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4fNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4fvNV);

	#endif // GL_NV_fragment_program_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_half_float ]--------------------------------------------------------------------------------------

//!	Initialize GL_NV_half_float.

bool CRenderingContext::InitNvHalfFloat()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_half_float_OGLEXT

		GET_PROC_ADDRESS(Color3hNV);
		GET_PROC_ADDRESS(Color3hvNV);
		GET_PROC_ADDRESS(Color4hNV);
		GET_PROC_ADDRESS(Color4hvNV);
		GET_PROC_ADDRESS(FogCoordhNV);
		GET_PROC_ADDRESS(FogCoordhvNV);
		GET_PROC_ADDRESS(MultiTexCoord1hNV);
		GET_PROC_ADDRESS(MultiTexCoord1hvNV);
		GET_PROC_ADDRESS(MultiTexCoord2hNV);
		GET_PROC_ADDRESS(MultiTexCoord2hvNV);
		GET_PROC_ADDRESS(MultiTexCoord3hNV);
		GET_PROC_ADDRESS(MultiTexCoord3hvNV);
		GET_PROC_ADDRESS(MultiTexCoord4hNV);
		GET_PROC_ADDRESS(MultiTexCoord4hvNV);
		GET_PROC_ADDRESS(Normal3hNV);
		GET_PROC_ADDRESS(Normal3hvNV);
		GET_PROC_ADDRESS(SecondaryColor3hNV);
		GET_PROC_ADDRESS(SecondaryColor3hvNV);
		GET_PROC_ADDRESS(TexCoord1hNV);
		GET_PROC_ADDRESS(TexCoord1hvNV);
		GET_PROC_ADDRESS(TexCoord2hNV);
		GET_PROC_ADDRESS(TexCoord2hvNV);
		GET_PROC_ADDRESS(TexCoord3hNV);
		GET_PROC_ADDRESS(TexCoord3hvNV);
		GET_PROC_ADDRESS(TexCoord4hNV);
		GET_PROC_ADDRESS(TexCoord4hvNV);
		GET_PROC_ADDRESS(Vertex2hNV);
		GET_PROC_ADDRESS(Vertex2hvNV);
		GET_PROC_ADDRESS(Vertex3hNV);
		GET_PROC_ADDRESS(Vertex3hvNV);
		GET_PROC_ADDRESS(Vertex4hNV);
		GET_PROC_ADDRESS(Vertex4hvNV);
		GET_PROC_ADDRESS(VertexAttrib1hNV);
		GET_PROC_ADDRESS(VertexAttrib1hvNV);
		GET_PROC_ADDRESS(VertexAttrib2hNV);
		GET_PROC_ADDRESS(VertexAttrib2hvNV);
		GET_PROC_ADDRESS(VertexAttrib3hNV);
		GET_PROC_ADDRESS(VertexAttrib3hvNV);
		GET_PROC_ADDRESS(VertexAttrib4hNV);
		GET_PROC_ADDRESS(VertexAttrib4hvNV);
		GET_PROC_ADDRESS(VertexAttribs1hvNV);
		GET_PROC_ADDRESS(VertexAttribs2hvNV);
		GET_PROC_ADDRESS(VertexAttribs3hvNV);
		GET_PROC_ADDRESS(VertexAttribs4hvNV);
		GET_PROC_ADDRESS(VertexWeighthNV);
		GET_PROC_ADDRESS(VertexWeighthvNV);

	#endif // GL_NV_half_float_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_occlusion_query ]---------------------------------------------------------------------------------

//!	Initialize GL_NV_occlusion_query.

bool CRenderingContext::InitNvOcclusionQuery()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_occlusion_query_OGLEXT

		GET_PROC_ADDRESS(BeginOcclusionQueryNV);
		GET_PROC_ADDRESS(DeleteOcclusionQueriesNV);
		GET_PROC_ADDRESS(EndOcclusionQueryNV);
		GET_PROC_ADDRESS(GenOcclusionQueriesNV);
		GET_PROC_ADDRESS(GetOcclusionQueryivNV);
		GET_PROC_ADDRESS(GetOcclusionQueryuivNV);
		GET_PROC_ADDRESS(IsOcclusionQueryNV);

	#endif // GL_NV_occlusion_query_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_pixel_data_range ]--------------------------------------------------------------------------------

//!	Initialize GL_NV_pixel_data_range.

bool CRenderingContext::InitNvPixelDataRange()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_pixel_data_range_OGLEXT

		GET_PROC_ADDRESS(FlushPixelDataRangeNV);
		GET_PROC_ADDRESS(PixelDataRangeNV);

	#endif // GL_NV_pixel_data_range_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_point_sprite ]------------------------------------------------------------------------------------

//!	Initialize GL_NV_point_sprite.

bool CRenderingContext::InitNvPointSprite()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_point_sprite_OGLEXT

		GET_PROC_ADDRESS(PointParameteriNV);
		GET_PROC_ADDRESS(PointParameterivNV);

	#endif // GL_NV_point_sprite_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_primitive_restart ]-------------------------------------------------------------------------------

//!	Initialize GL_NV_primitive_restart.

bool CRenderingContext::InitNvPrimitiveRestart()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_primitive_restart_OGLEXT

		GET_PROC_ADDRESS(PrimitiveRestartIndexNV);
		GET_PROC_ADDRESS(PrimitiveRestartNV);

	#endif // GL_NV_primitive_restart_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_register_combiners ]------------------------------------------------------------------------------

//!	Initialize GL_NV_register_combiners.

bool CRenderingContext::InitNvRegisterCombiners()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_register_combiners_OGLEXT

		GET_PROC_ADDRESS(CombinerInputNV);
		GET_PROC_ADDRESS(CombinerOutputNV);
		GET_PROC_ADDRESS(CombinerParameterfNV);
		GET_PROC_ADDRESS(CombinerParameterfvNV);
		GET_PROC_ADDRESS(CombinerParameteriNV);
		GET_PROC_ADDRESS(CombinerParameterivNV);
		GET_PROC_ADDRESS(FinalCombinerInputNV);
		GET_PROC_ADDRESS(GetCombinerInputParameterfvNV);
		GET_PROC_ADDRESS(GetCombinerInputParameterivNV);
		GET_PROC_ADDRESS(GetCombinerOutputParameterfvNV);
		GET_PROC_ADDRESS(GetCombinerOutputParameterivNV);
		GET_PROC_ADDRESS(GetFinalCombinerInputParameterfvNV);
		GET_PROC_ADDRESS(GetFinalCombinerInputParameterivNV);

	#endif // GL_NV_register_combiners_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_register_combiners2 ]-----------------------------------------------------------------------------

//!	Initialize GL_NV_register_combiners2.

bool CRenderingContext::InitNvRegisterCombiners2()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_register_combiners2_OGLEXT

		GET_PROC_ADDRESS(CombinerStageParameterfvNV);
		GET_PROC_ADDRESS(GetCombinerStageParameterfvNV);

	#endif // GL_NV_register_combiners2_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_stencil_two_side ]--------------------------------------------------------------------------------

//!	Initialize GL_NV_stencil_two_side.

bool CRenderingContext::InitNvStencilTwoSide()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_stencil_two_side_OGLEXT

		GET_PROC_ADDRESS(ActiveStencilFaceNV);

	#endif // GL_NV_stencil_two_side_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_vertex_array_range ]------------------------------------------------------------------------------

//!	Initialize GL_NV_vertex_array_range.

bool CRenderingContext::InitNvVertexArrayRange()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_vertex_array_range_OGLEXT

		GET_PROC_ADDRESS(FlushVertexArrayRangeNV);
		GET_PROC_ADDRESS(VertexArrayRangeNV);

	#endif // GL_NV_vertex_array_range_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_vertex_program ]----------------------------------------------------------------------------------

//!	Initialize GL_NV_vertex_program.

bool CRenderingContext::InitNvVertexProgram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_vertex_program_OGLEXT

		GET_PROC_ADDRESS(AreProgramsResidentNV);
		GET_PROC_ADDRESS(BindProgramNV);
		GET_PROC_ADDRESS(DeleteProgramsNV);
		GET_PROC_ADDRESS(ExecuteProgramNV);
		GET_PROC_ADDRESS(GenProgramsNV);
		GET_PROC_ADDRESS(GetProgramivNV);
		GET_PROC_ADDRESS(GetProgramParameterdvNV);
		GET_PROC_ADDRESS(GetProgramParameterfvNV);
		GET_PROC_ADDRESS(GetProgramStringNV);
		GET_PROC_ADDRESS(GetTrackMatrixivNV);
		GET_PROC_ADDRESS(GetVertexAttribdvNV);
		GET_PROC_ADDRESS(GetVertexAttribfvNV);
		GET_PROC_ADDRESS(GetVertexAttribivNV);
		GET_PROC_ADDRESS(GetVertexAttribPointervNV);
		GET_PROC_ADDRESS(IsProgramNV);
		GET_PROC_ADDRESS(LoadProgramNV);
		GET_PROC_ADDRESS(ProgramParameter4dNV);
		GET_PROC_ADDRESS(ProgramParameter4dvNV);
		GET_PROC_ADDRESS(ProgramParameter4fNV);
		GET_PROC_ADDRESS(ProgramParameter4fvNV);
		GET_PROC_ADDRESS(ProgramParameters4dvNV);
		GET_PROC_ADDRESS(ProgramParameters4fvNV);
		GET_PROC_ADDRESS(RequestResidentProgramsNV);
		GET_PROC_ADDRESS(TrackMatrixNV);
		GET_PROC_ADDRESS(VertexAttrib1dNV);
		GET_PROC_ADDRESS(VertexAttrib1dvNV);
		GET_PROC_ADDRESS(VertexAttrib1fNV);
		GET_PROC_ADDRESS(VertexAttrib1fvNV);
		GET_PROC_ADDRESS(VertexAttrib1sNV);
		GET_PROC_ADDRESS(VertexAttrib1svNV);
		GET_PROC_ADDRESS(VertexAttrib2dNV);
		GET_PROC_ADDRESS(VertexAttrib2dvNV);
		GET_PROC_ADDRESS(VertexAttrib2fNV);
		GET_PROC_ADDRESS(VertexAttrib2fvNV);
		GET_PROC_ADDRESS(VertexAttrib2sNV);
		GET_PROC_ADDRESS(VertexAttrib2svNV);
		GET_PROC_ADDRESS(VertexAttrib3dNV);
		GET_PROC_ADDRESS(VertexAttrib3dvNV);
		GET_PROC_ADDRESS(VertexAttrib3fNV);
		GET_PROC_ADDRESS(VertexAttrib3fvNV);
		GET_PROC_ADDRESS(VertexAttrib3sNV);
		GET_PROC_ADDRESS(VertexAttrib3svNV);
		GET_PROC_ADDRESS(VertexAttrib4dNV);
		GET_PROC_ADDRESS(VertexAttrib4dvNV);
		GET_PROC_ADDRESS(VertexAttrib4fNV);
		GET_PROC_ADDRESS(VertexAttrib4fvNV);
		GET_PROC_ADDRESS(VertexAttrib4sNV);
		GET_PROC_ADDRESS(VertexAttrib4svNV);
		GET_PROC_ADDRESS(VertexAttrib4ubNV);
		GET_PROC_ADDRESS(VertexAttrib4ubvNV);
		GET_PROC_ADDRESS(VertexAttribPointerNV);
		GET_PROC_ADDRESS(VertexAttribs1dvNV);
		GET_PROC_ADDRESS(VertexAttribs1fvNV);
		GET_PROC_ADDRESS(VertexAttribs1svNV);
		GET_PROC_ADDRESS(VertexAttribs2dvNV);
		GET_PROC_ADDRESS(VertexAttribs2fvNV);
		GET_PROC_ADDRESS(VertexAttribs2svNV);
		GET_PROC_ADDRESS(VertexAttribs3dvNV);
		GET_PROC_ADDRESS(VertexAttribs3fvNV);
		GET_PROC_ADDRESS(VertexAttribs3svNV);
		GET_PROC_ADDRESS(VertexAttribs4dvNV);
		GET_PROC_ADDRESS(VertexAttribs4fvNV);
		GET_PROC_ADDRESS(VertexAttribs4svNV);
		GET_PROC_ADDRESS(VertexAttribs4ubvNV);

	#endif // GL_NV_vertex_program_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NVX_conditional_render ]-----------------------------------------------------------------------------

//!	Initialize GL_NVX_conditional_render.

bool CRenderingContext::InitNvxConditionalRender()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NVX_conditional_render_OGLEXT

		GET_PROC_ADDRESS(BeginConditionalRenderNVX);
		GET_PROC_ADDRESS(EndConditionalRenderNVX);

	#endif // GL_NVX_conditional_render_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_PGI_misc_hints ]-------------------------------------------------------------------------------------

//!	Initialize GL_PGI_misc_hints.

bool CRenderingContext::InitPgiMiscHints()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_PGI_misc_hints_OGLEXT

		GET_PROC_ADDRESS(HintPGI);

	#endif // GL_PGI_misc_hints_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGI_color_table ]------------------------------------------------------------------------------------

//!	Initialize GL_SGI_color_table.

bool CRenderingContext::InitSgiColorTable()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGI_color_table_OGLEXT

		GET_PROC_ADDRESS(ColorTableParameterfvSGI);
		GET_PROC_ADDRESS(ColorTableParameterivSGI);
		GET_PROC_ADDRESS(ColorTableSGI);
		GET_PROC_ADDRESS(CopyColorTableSGI);
		GET_PROC_ADDRESS(GetColorTableParameterfvSGI);
		GET_PROC_ADDRESS(GetColorTableParameterivSGI);
		GET_PROC_ADDRESS(GetColorTableSGI);

	#endif // GL_SGI_color_table_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_detail_texture ]--------------------------------------------------------------------------------

//!	Initialize GL_SGIS_detail_texture.

bool CRenderingContext::InitSgisDetailTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_detail_texture_OGLEXT

		GET_PROC_ADDRESS(DetailTexFuncSGIS);
		GET_PROC_ADDRESS(GetDetailTexFuncSGIS);

	#endif // GL_SGIS_detail_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_fog_function ]----------------------------------------------------------------------------------

//!	Initialize GL_SGIS_fog_function.

bool CRenderingContext::InitSgisFogFunction()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_fog_function_OGLEXT

		GET_PROC_ADDRESS(FogFuncSGIS);
		GET_PROC_ADDRESS(GetFogFuncSGIS);

	#endif // GL_SGIS_fog_function_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_multisample ]-----------------------------------------------------------------------------------

//!	Initialize GL_SGIS_multisample.

bool CRenderingContext::InitSgisMultisample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_multisample_OGLEXT

		GET_PROC_ADDRESS(SampleMaskSGIS);
		GET_PROC_ADDRESS(SamplePatternSGIS);

	#endif // GL_SGIS_multisample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_pixel_texture ]---------------------------------------------------------------------------------

//!	Initialize GL_SGIS_pixel_texture.

bool CRenderingContext::InitSgisPixelTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_pixel_texture_OGLEXT

		GET_PROC_ADDRESS(GetPixelTexGenParameterfvSGIS);
		GET_PROC_ADDRESS(GetPixelTexGenParameterivSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameterfSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameterfvSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameteriSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameterivSGIS);

	#endif // GL_SGIS_pixel_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_point_parameters ]------------------------------------------------------------------------------

//!	Initialize GL_SGIS_point_parameters.

bool CRenderingContext::InitSgisPointParameters()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_point_parameters_OGLEXT

		GET_PROC_ADDRESS(PointParameterfSGIS);
		GET_PROC_ADDRESS(PointParameterfvSGIS);

	#endif // GL_SGIS_point_parameters_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_sharpen_texture ]-------------------------------------------------------------------------------

//!	Initialize GL_SGIS_sharpen_texture.

bool CRenderingContext::InitSgisSharpenTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_sharpen_texture_OGLEXT

		GET_PROC_ADDRESS(GetSharpenTexFuncSGIS);
		GET_PROC_ADDRESS(SharpenTexFuncSGIS);

	#endif // GL_SGIS_sharpen_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture4D ]-------------------------------------------------------------------------------------

//!	Initialize GL_SGIS_texture4D.

bool CRenderingContext::InitSgisTexture4d()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture4D_OGLEXT

		GET_PROC_ADDRESS(TexImage4DSGIS);
		GET_PROC_ADDRESS(TexSubImage4DSGIS);

	#endif // GL_SGIS_texture4D_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture_color_mask ]----------------------------------------------------------------------------

//!	Initialize GL_SGIS_texture_color_mask.

bool CRenderingContext::InitSgisTextureColorMask()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture_color_mask_OGLEXT

		GET_PROC_ADDRESS(TextureColorMaskSGIS);

	#endif // GL_SGIS_texture_color_mask_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture_filter4 ]-------------------------------------------------------------------------------

//!	Initialize GL_SGIS_texture_filter4.

bool CRenderingContext::InitSgisTextureFilter4()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture_filter4_OGLEXT

		GET_PROC_ADDRESS(GetTexFilterFuncSGIS);
		GET_PROC_ADDRESS(TexFilterFuncSGIS);

	#endif // GL_SGIS_texture_filter4_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_async ]-----------------------------------------------------------------------------------------

//!	Initialize GL_SGIX_async.

bool CRenderingContext::InitSgixAsync()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_async_OGLEXT

		GET_PROC_ADDRESS(AsyncMarkerSGIX);
		GET_PROC_ADDRESS(DeleteAsyncMarkersSGIX);
		GET_PROC_ADDRESS(FinishAsyncSGIX);
		GET_PROC_ADDRESS(GenAsyncMarkersSGIX);
		GET_PROC_ADDRESS(IsAsyncMarkerSGIX);
		GET_PROC_ADDRESS(PollAsyncSGIX);

	#endif // GL_SGIX_async_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_flush_raster ]----------------------------------------------------------------------------------

//!	Initialize GL_SGIX_flush_raster.

bool CRenderingContext::InitSgixFlushRaster()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_flush_raster_OGLEXT

		GET_PROC_ADDRESS(FlushRasterSGIX);

	#endif // GL_SGIX_flush_raster_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_fragment_lighting ]-----------------------------------------------------------------------------

//!	Initialize GL_SGIX_fragment_lighting.

bool CRenderingContext::InitSgixFragmentLighting()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_fragment_lighting_OGLEXT

		GET_PROC_ADDRESS(FragmentColorMaterialSGIX);
		GET_PROC_ADDRESS(FragmentLightfSGIX);
		GET_PROC_ADDRESS(FragmentLightfvSGIX);
		GET_PROC_ADDRESS(FragmentLightiSGIX);
		GET_PROC_ADDRESS(FragmentLightivSGIX);
		GET_PROC_ADDRESS(FragmentLightModelfSGIX);
		GET_PROC_ADDRESS(FragmentLightModelfvSGIX);
		GET_PROC_ADDRESS(FragmentLightModeliSGIX);
		GET_PROC_ADDRESS(FragmentLightModelivSGIX);
		GET_PROC_ADDRESS(FragmentMaterialfSGIX);
		GET_PROC_ADDRESS(FragmentMaterialfvSGIX);
		GET_PROC_ADDRESS(FragmentMaterialiSGIX);
		GET_PROC_ADDRESS(FragmentMaterialivSGIX);
		GET_PROC_ADDRESS(GetFragmentLightfvSGIX);
		GET_PROC_ADDRESS(GetFragmentLightivSGIX);
		GET_PROC_ADDRESS(GetFragmentMaterialfvSGIX);
		GET_PROC_ADDRESS(GetFragmentMaterialivSGIX);
		GET_PROC_ADDRESS(LightEnviSGIX);

	#endif // GL_SGIX_fragment_lighting_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_framezoom ]-------------------------------------------------------------------------------------

//!	Initialize GL_SGIX_framezoom.

bool CRenderingContext::InitSgixFramezoom()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_framezoom_OGLEXT

		GET_PROC_ADDRESS(FrameZoomSGIX);

	#endif // GL_SGIX_framezoom_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_igloo_interface ]-------------------------------------------------------------------------------

//!	Initialize GL_SGIX_igloo_interface.

bool CRenderingContext::InitSgixIglooInterface()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_igloo_interface_OGLEXT

		GET_PROC_ADDRESS(IglooInterfaceSGIX);

	#endif // GL_SGIX_igloo_interface_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_instruments ]-----------------------------------------------------------------------------------

//!	Initialize GL_SGIX_instruments.

bool CRenderingContext::InitSgixInstruments()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_instruments_OGLEXT

		GET_PROC_ADDRESS(GetInstrumentsSGIX);
		GET_PROC_ADDRESS(InstrumentsBufferSGIX);
		GET_PROC_ADDRESS(PollInstrumentsSGIX);
		GET_PROC_ADDRESS(ReadInstrumentsSGIX);
		GET_PROC_ADDRESS(StartInstrumentsSGIX);
		GET_PROC_ADDRESS(StopInstrumentsSGIX);

	#endif // GL_SGIX_instruments_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_list_priority ]---------------------------------------------------------------------------------

//!	Initialize GL_SGIX_list_priority.

bool CRenderingContext::InitSgixListPriority()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_list_priority_OGLEXT

		GET_PROC_ADDRESS(GetListParameterfvSGIX);
		GET_PROC_ADDRESS(GetListParameterivSGIX);
		GET_PROC_ADDRESS(ListParameterfSGIX);
		GET_PROC_ADDRESS(ListParameterfvSGIX);
		GET_PROC_ADDRESS(ListParameteriSGIX);
		GET_PROC_ADDRESS(ListParameterivSGIX);

	#endif // GL_SGIX_list_priority_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_pixel_texture ]---------------------------------------------------------------------------------

//!	Initialize GL_SGIX_pixel_texture.

bool CRenderingContext::InitSgixPixelTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_pixel_texture_OGLEXT

		GET_PROC_ADDRESS(PixelTexGenSGIX);

	#endif // GL_SGIX_pixel_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_polynomial_ffd ]--------------------------------------------------------------------------------

//!	Initialize GL_SGIX_polynomial_ffd.

bool CRenderingContext::InitSgixPolynomialFfd()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_polynomial_ffd_OGLEXT

		GET_PROC_ADDRESS(DeformationMap3dSGIX);
		GET_PROC_ADDRESS(DeformationMap3fSGIX);
		GET_PROC_ADDRESS(DeformSGIX);
		GET_PROC_ADDRESS(LoadIdentityDeformationMapSGIX);

	#endif // GL_SGIX_polynomial_ffd_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_reference_plane ]-------------------------------------------------------------------------------

//!	Initialize GL_SGIX_reference_plane.

bool CRenderingContext::InitSgixReferencePlane()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_reference_plane_OGLEXT

		GET_PROC_ADDRESS(ReferencePlaneSGIX);

	#endif // GL_SGIX_reference_plane_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_sprite ]----------------------------------------------------------------------------------------

//!	Initialize GL_SGIX_sprite.

bool CRenderingContext::InitSgixSprite()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_sprite_OGLEXT

		GET_PROC_ADDRESS(SpriteParameterfSGIX);
		GET_PROC_ADDRESS(SpriteParameterfvSGIX);
		GET_PROC_ADDRESS(SpriteParameteriSGIX);
		GET_PROC_ADDRESS(SpriteParameterivSGIX);

	#endif // GL_SGIX_sprite_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_tag_sample_buffer ]-----------------------------------------------------------------------------

//!	Initialize GL_SGIX_tag_sample_buffer.

bool CRenderingContext::InitSgixTagSampleBuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_tag_sample_buffer_OGLEXT

		GET_PROC_ADDRESS(TagSampleBufferSGIX);

	#endif // GL_SGIX_tag_sample_buffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_global_alpha ]-----------------------------------------------------------------------------------

//!	Initialize GL_SUN_global_alpha.

bool CRenderingContext::InitSunGlobalAlpha()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_global_alpha_OGLEXT

		GET_PROC_ADDRESS(GlobalAlphaFactorbSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactordSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorfSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactoriSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorsSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorubSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactoruiSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorusSUN);

	#endif // GL_SUN_global_alpha_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_mesh_array ]-------------------------------------------------------------------------------------

//!	Initialize GL_SUN_mesh_array.

bool CRenderingContext::InitSunMeshArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_mesh_array_OGLEXT

		GET_PROC_ADDRESS(DrawMeshArraysSUN);

	#endif // GL_SUN_mesh_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_triangle_list ]----------------------------------------------------------------------------------

//!	Initialize GL_SUN_triangle_list.

bool CRenderingContext::InitSunTriangleList()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_triangle_list_OGLEXT

		GET_PROC_ADDRESS(ReplacementCodePointerSUN);
		GET_PROC_ADDRESS(ReplacementCodeubSUN);
		GET_PROC_ADDRESS(ReplacementCodeubvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiSUN);
		GET_PROC_ADDRESS(ReplacementCodeuivSUN);
		GET_PROC_ADDRESS(ReplacementCodeusSUN);
		GET_PROC_ADDRESS(ReplacementCodeusvSUN);

	#endif // GL_SUN_triangle_list_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_vertex ]-----------------------------------------------------------------------------------------

//!	Initialize GL_SUN_vertex.

bool CRenderingContext::InitSunVertex()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_vertex_OGLEXT

		GET_PROC_ADDRESS(Color3fVertex3fSUN);
		GET_PROC_ADDRESS(Color3fVertex3fvSUN);
		GET_PROC_ADDRESS(Color4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(Color4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(Color4ubVertex2fSUN);
		GET_PROC_ADDRESS(Color4ubVertex2fvSUN);
		GET_PROC_ADDRESS(Color4ubVertex3fSUN);
		GET_PROC_ADDRESS(Color4ubVertex3fvSUN);
		GET_PROC_ADDRESS(Normal3fVertex3fSUN);
		GET_PROC_ADDRESS(Normal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4ubVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4ubVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fColor3fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fColor3fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4ubVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4ubVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord4fColor4fNormal3fVertex4fSUN);
		GET_PROC_ADDRESS(TexCoord4fColor4fNormal3fVertex4fvSUN);
		GET_PROC_ADDRESS(TexCoord4fVertex4fSUN);
		GET_PROC_ADDRESS(TexCoord4fVertex4fvSUN);

	#endif // GL_SUN_vertex_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUNX_constant_data ]---------------------------------------------------------------------------------

//!	Initialize GL_SUNX_constant_data.

bool CRenderingContext::InitSunxConstantData()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUNX_constant_data_OGLEXT

		GET_PROC_ADDRESS(FinishTextureSUNX);

	#endif // GL_SUNX_constant_data_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


