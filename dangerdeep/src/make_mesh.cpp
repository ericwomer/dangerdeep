/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// helper functions to create meshes
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "make_mesh.h"
#include <cmath>

#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
#include <complex.h>
#ifndef isfinite
#define isfinite(x) finite(x)
#endif
#elif defined(WIN32)
#include <float.h>
#ifndef isfinite
#define isfinite(x) _finite(x)
#endif
#else
using std::isfinite;
#endif

using namespace std;

#include <cmath>

namespace make_mesh {

vector3f vec(int vi)
{
	switch (vi) {
	case -1: return vector3f(-1, 0, 0);
	case  1: return vector3f( 1, 0, 0);
	case -2: return vector3f( 0,-1, 0);
	case  2: return vector3f( 0, 1, 0);
	case -3: return vector3f( 0, 0,-1);
	case  3: return vector3f( 0, 0, 1);
	default: return vector3f();
	}
}

void quadface(model::mesh* m, unsigned bv, float uscal, float vscal, bool out, int ni, int ti)
{
	unsigned bi = (bv/4)*6;
	if (out) {
		m->indices[bi+0] = bv+0;
		m->indices[bi+1] = bv+1;
		m->indices[bi+2] = bv+3;
		m->indices[bi+3] = bv+3;
		m->indices[bi+4] = bv+1;
		m->indices[bi+5] = bv+2;
	} else {
		ni = -ni;
		ti = -ti;
		m->indices[bi+0] = bv+3;
		m->indices[bi+1] = bv+2;
		m->indices[bi+2] = bv+0;
		m->indices[bi+3] = bv+0;
		m->indices[bi+4] = bv+2;
		m->indices[bi+5] = bv+1;
	}
	m->texcoords[bv+0] = vector2f(0, 0);
	m->texcoords[bv+1] = vector2f(0, vscal);
	m->texcoords[bv+2] = vector2f(uscal, vscal);
	m->texcoords[bv+3] = vector2f(uscal, 0);
	m->normals[bv+0] = m->normals[bv+1] = m->normals[bv+2] = m->normals[bv+3] = vec(ni);
	m->tangentsx[bv+0] = m->tangentsx[bv+1] = m->tangentsx[bv+2] = m->tangentsx[bv+3] = vec(ti);
	m->righthanded[bv+0] = m->righthanded[bv+1] = m->righthanded[bv+2] = m->righthanded[bv+3] = true;
}


model::mesh* cube(float w, float l, float h, float uscal, float vscal, bool out, const string& name)
{
	model::mesh* m = new model::mesh();
	m->name = name;
	m->vertices.resize(6*4);
	m->indices.resize(6*2*3);
	m->texcoords.resize(6*4);
	m->normals.resize(6*4);
	m->tangentsx.resize(6*4);
	m->righthanded.resize(6*4);
	float w2 = w/2, l2 = l/2, h2 = h/2;
	int cnr[24] = {  3, 1, 0, 2,  7, 5, 1, 3,  6, 4, 5, 7,  2, 0, 4, 6,  2, 6, 7, 3,  4, 0, 1, 5 };
	for (unsigned j = 0; j < 24; ++j) {
		int i = cnr[j];
		m->vertices[j] = vector3f((i&1) ? w2 : -w2, (i&2) ? l2 : -l2, (i&4) ? h2 : -h2);
	}
	quadface(m,  0, uscal,vscal, out, -3, -1);
	quadface(m,  4, uscal,vscal, out,  1, -3);
	quadface(m,  8, uscal,vscal, out,  3,  1);
	quadface(m, 12, uscal,vscal, out, -1,  3);
	quadface(m, 16, uscal,vscal, out,  2,  1);
	quadface(m, 20, uscal,vscal, out, -2,  1);
	return m;
}

model::mesh* sphere(float radius, float height,
		 unsigned slices, unsigned stacks,
		 float uscal, float vscal,
		 bool out, const string& name)
{
	if (slices < 3) slices = 3;
	if (stacks < 2) stacks = 2;
	model::mesh* m = new model::mesh();
	m->name = name;

	unsigned nrvert = 2+(stacks-1)*(slices+1);
	m->vertices.resize(nrvert);
	m->indices.resize(2*(slices+1)*stacks+(stacks-1)*2);
	m->indices_type = model::mesh::pt_triangle_strip;
	m->texcoords.resize(nrvert);
	m->normals.resize(nrvert);
	m->tangentsx.resize(nrvert);
//fixme: righthanded info is missing!!!! maybe the reason for the display bugs
	m->righthanded.resize(nrvert, true);
	unsigned vptr = 2, iptr = 0;
	m->texcoords[0] = vector2f(0, 0);
	m->vertices[0] = vector3f(0, -height/2, 0);
	m->normals[0] = vector3f(0, out ? -1 : 1, 0);
	m->tangentsx[0] = vector3f(1, 0, 0);
	m->texcoords[1] = vector2f(0, vscal);
	m->vertices[1] = vector3f(0, height/2, 0);
	m->normals[1] = vector3f(0, out ? 1 : -1, 0);
	m->tangentsx[1] = vector3f(1, 0, 0);
	for (unsigned y = 1; y < stacks; y++) {
		float yc = -cos(y*M_PI/stacks) * height/2.0;
		float yr = sin(y*M_PI/stacks) * radius;
		for (unsigned x = 0; x <= slices; x++) {
			float xr = cos(x*M_PI*2/slices);
			float zr = sin(x*M_PI*2/slices);
			m->texcoords[vptr] = vector2f(uscal*float(x)/slices,
						     vscal*float(y)/stacks);
			m->vertices[vptr] = vector3f(xr * yr, yc, zr * yr);
			m->normals[vptr] = m->vertices[vptr].normal();
			if (!out) m->normals[vptr] = -m->normals[vptr];
			m->tangentsx[vptr] = vector3f(-zr * yr, yc, xr * yr);
			++vptr;
		}
	}
	// lowest stack
	for (unsigned x = 0; x <= slices; x++) {
		unsigned xx = out ? slices-x : x;
		m->indices[iptr+0] = 2 + xx;
		m->indices[iptr+1] = 0;
		iptr += 2;
	}
	m->indices[iptr] = m->indices[iptr-1];
	++iptr;
	// middle part
	for (unsigned y = 0; y < stacks-2; y++) {
		m->indices[iptr] = 2 + (out ? slices : 0) + (y+1)*(slices+1);
		++iptr;
		for (unsigned x = 0; x <= slices; x++) {
			unsigned xx = out ? slices-x : x;
			m->indices[iptr+0] = 2 + xx + (y+1)*(slices+1);
			m->indices[iptr+1] = 2 + xx +  y   *(slices+1);
			iptr += 2;
		}
		m->indices[iptr] = m->indices[iptr-1];
		++iptr;
	}
	// highest stack
	m->indices[iptr] = 1;
	++iptr;
	for (unsigned x = 0; x <= slices; x++) {
		unsigned xx = out ? slices-x : x;
		m->indices[iptr+0] = 1;
		m->indices[iptr+1] = 2 + xx + (stacks-2)*(slices+1);
		iptr += 2;
	}
	return m;
}

model::mesh* cylinder(float r, float h, unsigned rsegs,
		   float uscal, float vscal,
		   bool cap, bool out,//fixme: out seems to be ignored!
		   const string& name)
{
	return cone(r, r, h, rsegs, uscal, vscal, cap, out, name);
}

model::mesh* cone(float r0, float r1, float h, unsigned rsegs,
	       float uscal, float vscal,
	       bool cap, bool out,
	       const string& name)
{
	//fixme: out?, fixme cap map falsch, rest auch checken
	if (rsegs < 3) rsegs = 3;
	model::mesh* m = new model::mesh();
	m->name = name;

	unsigned nrvert = 2*(rsegs+1);
	unsigned basecap0 = nrvert, basecap1 = nrvert;
	unsigned basei0 = rsegs*6, basei1 = rsegs*6;
	if (cap && r0 > 0) {
		nrvert += (rsegs+1)+1;
		basecap1 = nrvert;
		basei1 += rsegs*3;
	}
	if (cap && r1 > 0) {
		nrvert += (rsegs+1)+1;
	}
	m->vertices.resize(nrvert);
	m->texcoords.resize(nrvert);
	m->normals.resize(nrvert);
	m->tangentsx.resize(nrvert);
	unsigned nrindis = rsegs*2*3;
	if (cap && r0 > 0) nrindis += rsegs*3;
	if (cap && r1 > 0) nrindis += rsegs*3;
	m->indices.resize(nrindis);

	for (unsigned i = 0; i <= rsegs; ++i) {
		float ang = 2*M_PI*i/rsegs;
		float sa = sin(ang);
		float ca = cos(ang);
		m->vertices[i] = vector3f(r0*ca, r0*sa, -h/2);
		m->vertices[i+(rsegs+1)] = vector3f(r1*ca, r1*sa, h/2);
		m->normals[i] = m->normals[i+(rsegs+1)] = vector3f(ca, sa, (r0-r1)/h).normal();
		m->tangentsx[i] = m->tangentsx[i+(rsegs+1)] = vector3f(-sa, ca, 0);
		m->texcoords[i] = vector2f(uscal*i/float(rsegs), vscal);
		m->texcoords[i+(rsegs+1)] = vector2f(uscal*i/float(rsegs), 0);
		if (cap && r0 > 0) {
			//fixme: tangente spiegeln?
			m->vertices[i+basecap0] = m->vertices[i];
			m->normals[i+basecap0] = vector3f(0,0,-1);
			m->texcoords[i+basecap0] = vector2f(0.5f+0.5f*ca, 0.5f+0.5f*sa);
			m->tangentsx[i+basecap0] = vector3f(1,0,0);
		}
		if (cap && r1 > 0) {
			m->vertices[i+basecap1] = m->vertices[i+(rsegs+1)];
			m->normals[i+basecap1] = vector3f(0,0,1);
			m->texcoords[i+basecap1] = vector2f(0.5f+0.5f*ca, 0.5f+0.5f*sa);
			m->tangentsx[i+basecap1] = vector3f(1,0,0);
		}
	}
	if (cap && r0 > 0) {
		m->vertices[basecap0+(rsegs+1)] = vector3f(0,0,-h/2);
		m->normals[basecap0+(rsegs+1)] = vector3f(0,0,-1);
		m->texcoords[basecap0+(rsegs+1)] = vector2f(0.5f,0.5f);
		m->tangentsx[basecap0+(rsegs+1)] = vector3f(1,0,0);
	}
	if (cap && r1 > 0) {
		m->vertices[basecap1+(rsegs+1)] = vector3f(0,0,h/2);
		m->normals[basecap1+(rsegs+1)] = vector3f(0,0,1);
		m->texcoords[basecap1+(rsegs+1)] = vector2f(0.5f,0.5f);
		m->tangentsx[basecap1+(rsegs+1)] = vector3f(1,0,0);
	}

	for (unsigned j = 0; j < rsegs; ++j) {
		m->indices[j*6] = j;
		m->indices[j*6+1] = m->indices[j*6+4] = j+1;
		m->indices[j*6+2] = m->indices[j*6+3] = j+(rsegs+1);
		m->indices[j*6+5] = j+1+(rsegs+1);
		if (cap && r0 > 0) {
			m->indices[basei0+j*3] = basecap0+j+1;
			m->indices[basei0+j*3+1] = basecap0+j;
			m->indices[basei0+j*3+2] = basecap0+(rsegs+1);
		}
		if (cap && r1 > 0) {
			m->indices[basei1+j*3] = basecap1+j;
			m->indices[basei1+j*3+1] = basecap1+j+1;
			m->indices[basei1+j*3+2] = basecap1+(rsegs+1);
		}
	}
	return m;
}

model::mesh* torus(float outerr, float innerr, unsigned outerrsegs,
		unsigned innerrsegs, float uscal, float vscal, bool out,
		const string& name)
{
	if (outerrsegs < 3) outerrsegs = 3;
	if (innerrsegs < 3) innerrsegs = 3;
	model::mesh* m = new model::mesh();
	m->name = name;
	unsigned nrvert = (outerrsegs+1)*(innerrsegs+1);
	m->vertices.resize(nrvert);
	m->texcoords.resize(nrvert);
	m->normals.resize(nrvert);
	m->tangentsx.resize(nrvert);
	m->indices.resize(outerrsegs*innerrsegs*6);
	for (unsigned i = 0; i <= outerrsegs; ++i) {
		float ang = 2*M_PI*i/outerrsegs;
		float sa = sin(ang);
		float ca = cos(ang);
		for (unsigned j = 0; j <= innerrsegs; ++j) {
			float iang = 2*M_PI*j/innerrsegs;
			float isa = sin(iang);
			float ica = cos(iang);
			m->vertices[i*(innerrsegs+1)+j] = vector3f(ca*(outerr+ica*innerr),
								  sa*(outerr+ica*innerr),
								  isa*innerr);
			m->texcoords[i*(innerrsegs+1)+j] = vector2f(uscal*j/innerrsegs,
								   vscal*i/outerrsegs);
			m->normals[i*(innerrsegs+1)+j] = vector3f(ca*ica, sa*ica, isa);
			m->tangentsx[i*(innerrsegs+1)+j] = vector3f(-sa, ca, 0);
		}
	}
	for (unsigned i = 0; i < outerrsegs; ++i) {
		for (unsigned j = 0; j < innerrsegs; ++j) {
			m->indices[(i*innerrsegs+j)*6] = i*(innerrsegs+1)+j;
			m->indices[(i*innerrsegs+j)*6+2] = m->indices[(i*innerrsegs+j)*6+3] =
				i*(innerrsegs+1)+j+1;
			m->indices[(i*innerrsegs+j)*6+1] = m->indices[(i*innerrsegs+j)*6+4] =
				(i+1)*(innerrsegs+1)+j;
			m->indices[(i*innerrsegs+j)*6+5] = (i+1)*(innerrsegs+1)+j+1;
		}
	}
	return m;
}

/*
model::mesh spiral(float r, float h, float outerr, unsigned rsegs = 16,
				     unsigned hsegs = 16, bool out = true, const string& name);

model::mesh part_of_torus(float outerr, float innerr,
					    unsigned outerrseggen = 8,
					    unsigned outerrsegs = 32,
					    unsigned innerrsegs = 16,
					    bool cap = true, bool out = true, const string& name);
*/


model::mesh* heightfield(unsigned resx, unsigned resy, const vector<Uint8>& heights,
			     float xscal, float yscal, float zscal,
			     float xnoise, float ynoise, float znoise,
			     const string& name)
{
	if (resx < 2 || resy < 2 || heights.size() != resx*resy) return 0;
	model::mesh* m = new model::mesh();
	m->name = name;
	unsigned nrvert = resx*resy;
	m->vertices.resize(nrvert);
	m->texcoords.resize(nrvert);
	m->normals.resize(nrvert);
	m->tangentsx.resize(nrvert);
	m->indices.resize((resx-1)*(resy-1)*2*3);
	for (unsigned i = 0; i < resy; ++i) {
		for (unsigned j = 0; j < resx; ++j) {
			float r0 = float(rand())/RAND_MAX;
			float r1 = float(rand())/RAND_MAX;
			float r2 = float(rand())/RAND_MAX;
			m->vertices[i*resx+j] = vector3f(xscal * j + (r0 * 2 - 1) * xnoise,
							yscal * i + (r1 * 2 - 1) * ynoise,
							zscal * float(heights[i*resx+j]) *
							(1.0f/255) + (r2 * 2 - 1) * znoise);
			// mapping is always straight, even with noise
			m->texcoords[i*resx+j] = vector2f(float(j)/(resx-1), float(i)/(resy-1));
			// normals are computed later
			m->tangentsx[i*resx+j] = vector3f(1, 0, 0);
		}
	}
	for (unsigned k = 0; k < resy-1; ++k) {
		for (unsigned j = 0; j < resx-1; ++j) {
			unsigned ib = ((resx-1)*k+j)*6;
			m->indices[ib] = k*resx+j;
			m->indices[ib+1] = m->indices[ib+4] = k*resx+j+1;
			m->indices[ib+2] = m->indices[ib+3] = (k+1)*resx+j;
			m->indices[ib+5] = (k+1)*resx+j+1;

			// compute normals
			const vector3f& v0 = m->vertices[m->indices[ib]];
			const vector3f& v1 = m->vertices[m->indices[ib+1]];
			const vector3f& v2 = m->vertices[m->indices[ib+2]];
			const vector3f& v3 = m->vertices[m->indices[ib+5]];
			vector3f ortho = (v1-v0).orthogonal(v2-v0);
			float lf = 1.0/ortho.length();
			// icc fix, doesn't like the other method, gcc does something wierd with isfinite()
			if ((fpclassify(lf) != FP_NAN && fpclassify(lf) != FP_INFINITE)) {
				vector3f face_normal = ortho * lf;
				m->normals[m->indices[ib]] += face_normal;
				m->normals[m->indices[ib+1]] += face_normal;
				m->normals[m->indices[ib+2]] += face_normal;
			}
			ortho = (v1-v2).orthogonal(v3-v2);
			lf = 1.0/ortho.length();
			// icc fix, doesn't like the other method, gcc does something wierd with isfinite()
			if ((fpclassify(lf) != FP_NAN && fpclassify(lf) != FP_INFINITE)) {
				vector3f face_normal = ortho * lf;
				m->normals[m->indices[ib+3]] += face_normal;
				m->normals[m->indices[ib+4]] += face_normal;
				m->normals[m->indices[ib+5]] += face_normal;
			}
		}
	}

	for (vector<vector3f>::iterator it = m->normals.begin(); it != m->normals.end(); ++it) {
		it->normalize();
	}

	return m;
}

}//namespace
