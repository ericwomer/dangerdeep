// Macros.h                                                  Copyright (C) 2006 Thomas Jansen (jansen@caesar.de)
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

#ifndef	_OGL_MACROS_H_
#define	_OGL_MACROS_H_

#if !defined(_WIN32) && (!defined(__APPLE__) || !defined(__GNUC__)) && !defined(__MACOSX__)

	#include <GL/glx.h>

#endif


// ---[ INTERNAL API WRAPPER MACROS ]---------------------------------------------------------------------------

#if defined(_WIN32) && defined(_DEBUG)

	#define	WRAPPER_CALL_V(NAME)	CRenderingContext * pContext = g_RCHashArray.PrepareRenderingContext(); \
											if(!pContext || !pContext->m_p##NAME) {\
												::OutputDebugString("OglExt error: gl" #NAME "() unsupported!\n"); \
												return; \
											} \
											pContext->m_p##NAME

	#define	WRAPPER_CALL_R(NAME)	CRenderingContext * pContext = g_RCHashArray.PrepareRenderingContext(); \
											if(!pContext || !pContext->m_p##NAME) {\
												::OutputDebugString("OglExt error: gl" #NAME "() unsupported!\n"); \
												return 0; \
											} \
											return pContext->m_p##NAME

#else		// _WIN32 && _DEBUG

	#define	WRAPPER_CALL_V(NAME)	CRenderingContext * pContext = g_RCHashArray.PrepareRenderingContext(); \
											if(!pContext || !pContext->m_p##NAME) return; \
											pContext->m_p##NAME

	#define	WRAPPER_CALL_R(NAME)	CRenderingContext * pContext = g_RCHashArray.PrepareRenderingContext(); \
											if(!pContext || !pContext->m_p##NAME) return 0; \
											return pContext->m_p##NAME

#endif	// _WIN32 && _DEBUG


// ---[ EXTERNAL API WRAPPER MACROS ]---------------------------------------------------------------------------

	#define	WRAPPER00V(NAME) \
											gl##NAME () \
											{ \
												WRAPPER_CALL_V(NAME) (); \
											}

	#define	WRAPPER01V(NAME, T1) \
											gl##NAME (T1 v1) \
											{ \
												WRAPPER_CALL_V(NAME) (v1); \
											}

	#define	WRAPPER02V(NAME, T1, T2) \
											gl##NAME (T1 v1, T2 v2) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2); \
											}

	#define	WRAPPER03V(NAME, T1, T2, T3)	\
											gl##NAME (T1 v1, T2 v2, T3 v3) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3); \
											}

	#define	WRAPPER04V(NAME, T1, T2, T3, T4) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4); \
											}

	#define	WRAPPER05V(NAME, T1, T2, T3, T4, T5) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5); \
											}

	#define	WRAPPER06V(NAME, T1, T2, T3, T4, T5, T6) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6); \
											}

	#define	WRAPPER07V(NAME, T1, T2, T3, T4, T5, T6, T7) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7); \
											}

	#define	WRAPPER08V(NAME, T1, T2, T3, T4, T5, T6, T7, T8) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8); \
											}

	#define	WRAPPER09V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9); \
											}

	#define	WRAPPER10V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10); \
											}

	#define	WRAPPER11V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11); \
											}

	#define	WRAPPER12V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12); \
											}

	#define	WRAPPER13V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13); \
											}

	#define	WRAPPER14V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14); \
											}

	#define	WRAPPER15V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15); \
											}

	#define	WRAPPER16V(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15, T16 v16) \
											{ \
												WRAPPER_CALL_V(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16); \
											}


	#define	WRAPPER00R(NAME) \
											gl##NAME () \
											{ \
												WRAPPER_CALL_R(NAME) (); \
											}

	#define	WRAPPER01R(NAME, T1) \
											gl##NAME (T1 v1) \
											{ \
												WRAPPER_CALL_R(NAME) (v1); \
											}

	#define	WRAPPER02R(NAME, T1, T2) \
											gl##NAME (T1 v1, T2 v2) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2); \
											}

	#define	WRAPPER03R(NAME, T1, T2, T3)	\
											gl##NAME (T1 v1, T2 v2, T3 v3) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3); \
											}

	#define	WRAPPER04R(NAME, T1, T2, T3, T4) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4); \
											}

	#define	WRAPPER05R(NAME, T1, T2, T3, T4, T5) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5); \
											}

	#define	WRAPPER06R(NAME, T1, T2, T3, T4, T5, T6) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6); \
											}

	#define	WRAPPER07R(NAME, T1, T2, T3, T4, T5, T6, T7) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7); \
											}

	#define	WRAPPER08R(NAME, T1, T2, T3, T4, T5, T6, T7, T8) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8); \
											}

	#define	WRAPPER09R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9); \
											}

	#define	WRAPPER10R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10); \
											}

	#define	WRAPPER11R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11); \
											}

	#define	WRAPPER12R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12); \
											}

	#define	WRAPPER13R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13); \
											}

	#define	WRAPPER14R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14); \
											}

	#define	WRAPPER15R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15); \
											}

	#define	WRAPPER16R(NAME, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) \
											gl##NAME (T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15, T16 v16) \
											{ \
												WRAPPER_CALL_R(NAME) (v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16); \
											}


// ---[ MACROS TO DETERMINE API ENTRY POINTS ]------------------------------------------------------------------

#if defined(_WIN32) && defined(_DEBUG)

	#define	GET_PROC_ADDRESS(NAME)		*((void * *) &m_p##NAME ) = GetProcAddress( #NAME ); \
													if(m_p##NAME == NULL) { \
														::OutputDebugString("OglExt warning: Function \"" #NAME "\" not linkable!\n"); \
														bReturn = false; \
													}

#else		// _WIN32 && _DEBUG

	#define	GET_PROC_ADDRESS(NAME)		*((void * *) &m_p##NAME ) = GetProcAddress( #NAME ); \
													if(m_p##NAME == NULL) { \
														bReturn = false; \
													}

#endif	// _WIN32 && _DEBUG


// ---[ MACROS TO INITIALIZE AN EXTENSION OR A GL VERSION ]-----------------------------------------------------

#if defined(_WIN32) && defined(_DEBUG)

	#define	INIT_EXTENSION(NAME, FUNC) \
													if(IsExtensionSupported(NAME)) { \
														bool br = FUNC(); \
														if(!br) { \
															::OutputDebugString("==> OglExt warning: " NAME " supported but not linkable!\n"); \
														} \
													}

	#define	INIT_GLVERSION(MAJOR, MINOR, RELEASE, FUNC) \
													if(GetVersion() >= GLEX_VERSION(MAJOR, MINOR, RELEASE)) { \
														bool br = FUNC(); \
														if(!br) { \
															::OutputDebugString("OglExt warning: Version " #MAJOR "." #MINOR "." #RELEASE " supported but not linkable!\n"); \
														} \
													}

#else		// _WIN32 && _DEBUG

	#define	INIT_EXTENSION(NAME, FUNC) \
													if(IsExtensionSupported(NAME)) { \
														FUNC(); \
													}

	#define	INIT_GLVERSION(MAJOR, MINOR, RELEASE, FUNC) \
													if(GetVersion() >= GLEX_VERSION(MAJOR, MINOR, RELEASE)) { \
														FUNC(); \
													}
#endif	// _WIN32 && _DEBUG


#endif	// _OGL_MACROS_H_
