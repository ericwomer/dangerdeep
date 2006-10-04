// RCHashArray.hpp                                           Copyright (C) 2006 Thomas Jansen (jansen@caesar.de)
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

#ifndef	_OGL_RCHASHARRAY_HPP_
#define	_OGL_RCHASHARRAY_HPP_

#include	"RenderingContext.hpp"

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
	#define	WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
#endif

#if !defined(_WIN32) && (!defined(__APPLE__) || !defined(__GNUC__)) && !defined(__MACOSX__)
	#include <GL/glx.h>
#endif

#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
	#include <agl.h>
	#include <gl.h>
#else
	#include <GL/gl.h>
#endif



// ---[ TYPE DEFINITIONS ]--------------------------------------------------------------------------------------

typedef	void *	HRCKEY;


// =============================================================================================================
// ===                                   C L A S S   D E C L A R A T I O N                                   ===
// =============================================================================================================

//!	A really simple hash array class.

class CRCHashArray {

protected:

	// - -[ constants ] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum {

		HASH_SIZE					= 31														//!<               The hashing prime.
	};

	// - -[ structures ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	//!	List node for the hash array buckets.

	struct SHashArrayNode {

		SHashArrayNode *			pNext;													//!<        Pointer to the next node.
		HRCKEY						hRCKey;													//!<    The OpenGL rendering context.

		CRenderingContext *		pRenderingContext;									//!<           The rendering context.
	};

public:

	// - -[ class administration ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

										CRCHashArray();										//                    the constructor
									  ~CRCHashArray();										//                     the destructor

	// - -[ operations ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	#ifdef	_WIN32

		CRenderingContext *		GetRenderingContext(HRCKEY hRCKey = ::wglGetCurrentContext());			//  return rc
		CRenderingContext *		PrepareRenderingContext(HRCKEY hRCKey = ::wglGetCurrentContext());	//  create rc

	#elif (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)

		CRenderingContext *		GetRenderingContext(HRCKEY hRCKey = ::aglGetCurrentContext());			//  return rc
		CRenderingContext *		PrepareRenderingContext(HRCKEY hRCKey = ::aglGetCurrentContext());	//  create rc

	#else		// _WIN32

		CRenderingContext *		GetRenderingContext(HRCKEY hRCKey = ::glXGetCurrentContext());			//  return rc
		CRenderingContext *		PrepareRenderingContext(HRCKEY hRCKey = ::glXGetCurrentContext());	//  create rc

	#endif

protected:

	// - -[ tool methods ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	unsigned long					CalcHash(HRCKEY hRCKey);							//              calculate the rc hash

	// - -[ protected fields ]- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	SHashArrayNode *				m_pHashArray[HASH_SIZE];							//!<                  The hash array.

};


// =============================================================================================================
// ===                                        T O O L   M E T H O D S                                        ===
// =============================================================================================================

//!	Calculate the hash value.

inline unsigned long CRCHashArray::CalcHash(HRCKEY hRCKey)
{
	return (((unsigned long) hRCKey) % HASH_SIZE);
}


// =============================================================================================================
// ===                                C L A S S   A D M I N I S T R A T I O N                                ===
// =============================================================================================================

//!	The constructor.

inline CRCHashArray::CRCHashArray()
{
	// 1: initialize the hash array...

	for(unsigned long uIndex = 0; uIndex < HASH_SIZE; ++uIndex) {

		m_pHashArray[uIndex]	= NULL;
	}
}


//!	The destructor.

inline CRCHashArray::~CRCHashArray()
{
	// 1: go through all hash buckets...

	for(unsigned long uIndex = 0; uIndex < HASH_SIZE; ++uIndex) {

		// 1.1: delete a hash bucket...

		SHashArrayNode * pNode = m_pHashArray[uIndex];
		while(pNode) {

			// 1.1.1: fetch the next node...

			SHashArrayNode * pOldNode	= pNode;
			pNode								= pNode->pNext;

			// 1.1.2: free the memory...

			delete pOldNode->pRenderingContext;
			delete pOldNode;
		}
	}
}


// =============================================================================================================
// ===                                          O P E R A T I O N S                                          ===
// =============================================================================================================

//!	Return the rendering context for a given key.

inline CRenderingContext * CRCHashArray::GetRenderingContext(HRCKEY hRCKey)
{
	// 1: get the first node for the given hash bucket...

	SHashArrayNode * pNode = m_pHashArray[CalcHash(hRCKey)];

	while(pNode) {

		// 1.1: if we found the right context, return it...

		if(pNode->hRCKey == hRCKey) {

			return pNode->pRenderingContext;
		}

		// 1.2: go to next context in bucket...

		pNode = pNode->pNext;
	}

	// 2: nothing was found...

	return NULL;
}


//!	Return the rendering context for a given key (or create a new one if necessary).

inline CRenderingContext * CRCHashArray::PrepareRenderingContext(HRCKEY hRCKey)
{
	// 1: get the first node for the given hash bucket...

	unsigned long uHashIndex	= CalcHash(hRCKey);

	SHashArrayNode * pNode	= m_pHashArray[uHashIndex];

	while(pNode) {

		// 1.1: if we found the right context, return it...

		if(pNode->hRCKey == hRCKey) {

			return pNode->pRenderingContext;
		}

		// 1.2: go to next context in bucket...

		pNode = pNode->pNext;
	}

	// 2: if it's not a gl rendering context, we skip the rest...

	if(hRCKey == NULL) {

		return NULL;
	}

	// 3: create a new rendering context...

	SHashArrayNode * pNewNode		= new SHashArrayNode;

	pNewNode->pNext					= m_pHashArray[uHashIndex];
	pNewNode->hRCKey					= hRCKey;
	pNewNode->pRenderingContext	= new CRenderingContext;

	m_pHashArray[uHashIndex]		= pNewNode;

	// 4: return the new context...

	return pNewNode->pRenderingContext;
}


#endif	// _OGL_RCHASHARRAY_HPP_
