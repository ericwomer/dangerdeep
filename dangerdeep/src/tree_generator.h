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

// A 3d tree generator
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef TREE_GENERATOR_H
#define TREE_GENERATOR_H

#include "vector3.h"
#include "matrix3.h"
#include "matrix4.h"
#include "texture.h"
#include "color.h"
#include "shader.h"
#include "model.h"
#include "vertexbufferobject.h"
#include "bspline.h"
#include "global_data.h"
#include <vector>
#include <fstream>
#include <memory>
#include <map>
#include <set>

/*
Generiere Kegelstümpfe mit Biegung (bspline)
oberes ende mit kappe verschließen,
texturkoordinaten generieren, evtl.
auch wind-faktor (für bewegung), der ist am stamm 0 und wird immer größer bis zu den blättern.
stamm ist ein kegelstumpf (KS)
am ende 2-4 kinder, länge, winkel zufällig,
immer weiter verzweigen
äußerste mit blättern.
teilweise auch mittem am stamm (selten) neue äste abzweigen.
max. 4-8 KS in kette
parameter: anzahl vertices außenrum, in der höhe (für biegung) usw.
letzten vertex rundum doppelt für texturkoord.
generiere eine mesh für alles.
blätter sind dann dreiecke (mit alpha) oder 2 in diamantform (mit alpha, 
aber ohne z-sort).
textur: eine für alles, repeat, rinde nimmt aber nur unteren teil ein
also in höhe kein repeat durch texturkoord.
oder generell kein repeat.
oberer teil: verschiedene blätter.
gerne normal mapping oder sogar parallax/relief für nahe bäume.
zahl der dreiecke pro baum < 10000 am besten < 2000.
imposter für spätere.
auto-imposter-berechnung in class model implementieren.
tangentsx auch generieren
obwohl besser blätter extra, als einzelne quads, alle äste als EIN tri-strip
winkel der äste zu globaler z-axis wächst von 0 am stamm bis max. 90 am ende oder so
*/

class tree_generator
{
 public:
	tree_generator() {}
	std::auto_ptr<model::mesh> generate() const;
 protected:
	// bend: evtl. gebe besser axis unten und oben an, dazwischen einfach biegen
	void generate_log(model::mesh& msh,
			  const vector3f& root, const vector3f& axis, unsigned segs,
			  float length, float radius0, float radius1,
			  const vector3f& bend_dir = vector3f(0,0,0),
			  float bend_factor = 0.0f, unsigned bend_segs = 1) const;
	void generate_tree(model::mesh& msh, unsigned lvl,
			   const vector3f& root, const vector3f& axis) const;
};

std::auto_ptr<model::mesh> tree_generator::generate() const
{
	std::auto_ptr<model::mesh> msh(new model::mesh("tree"));
	generate_tree(*msh, 0, vector3f(0,0,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(3,0,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(0,3,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(3,3,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(0,6,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(6,6,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(6,0,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(6,3,0), vector3f(0,0,1));
	generate_tree(*msh, 0, vector3f(3,6,0), vector3f(0,0,1));
	msh->write_off_file("test.off");
	return msh;
}

void tree_generator::generate_log(model::mesh& msh,
				  const vector3f& root, const vector3f& axis, unsigned segs,
				  float length, float radius0, float radius1,
				  const vector3f& bend_dir,
				  float bend_factor, unsigned bend_segs) const
{
	if (segs < 3 || bend_segs < 1) throw std::invalid_argument("treegenerator::generate_log");
	const unsigned nr_verts = (segs+1) * (bend_segs+1);
	const unsigned nr_indis = (2*(segs+1) + 2) * bend_segs - (msh.indices.empty() ? 1 : 0);
	unsigned old_v = msh.vertices.size();
	unsigned old_i = msh.indices.size();
	msh.indices_type = model::mesh::pt_triangle_strip;
	msh.vertices.resize(old_v + nr_verts);
	msh.normals.resize(old_v + nr_verts);
	msh.tangentsx.resize(old_v + nr_verts);
	msh.texcoords.resize(old_v + nr_verts);
	msh.righthanded.resize(old_v + nr_verts, true);
	msh.indices.resize(old_i + nr_indis);
	// generate vertices
	vector3f xaxis(1, 0, 0); // fixme: if axis==xaxis?!
	vector3f yaxis = axis.cross(xaxis).normal();
	xaxis = yaxis.cross(axis);
	unsigned vidx = old_v;
	for (unsigned b = 0; b <= bend_segs; ++b) {
		float b_fac = float(b)/bend_segs;
		vector3f p = root + axis * (length * b_fac);
		float r = radius0 + (radius1 - radius0) * b_fac;
		// HIER xaxis berechnen! weil bend verändert das!
		for (unsigned s = 0; s <= segs; ++s) {
			float s_fac = float(s)/segs;
			float a = 2*M_PI * s_fac;
			vector2f d(cos(a), sin(a));
			msh.vertices[vidx] = p + xaxis * (d.x * r) + yaxis * (d.y * r);
			msh.normals[vidx] = (xaxis * d.x + yaxis * d.y).normal();
			msh.tangentsx[vidx] = (yaxis * d.x - xaxis * d.y).normal();
			msh.texcoords[vidx] = vector2f(s_fac, b_fac);
			++vidx;
		}
	}
	// generate indices
	unsigned iidx = old_i;
	for (unsigned b = 0; b < bend_segs; ++b) {
		unsigned v0 = old_v + b * (segs+1);
		unsigned v1 = v0 + segs+1;
		if (iidx > 0)
			msh.indices[iidx++] = v1;
		for (unsigned r = 0; r <= segs; ++r) {
			msh.indices[iidx++] = v1;
			msh.indices[iidx++] = v0;
			++v0;
			++v1;
		}
		msh.indices[iidx++] = v0-1;
	}
}

void tree_generator::generate_tree(model::mesh& msh, unsigned lvl,
				   const vector3f& root, const vector3f& axis) const
{
	static const unsigned segs[5] = { 16, 12, 8, 6, 3 };
	static const float length0[5] = { 1.0, 0.6, 0.4, 0.3, 0.15 };
	static const float length1[5] = { 1.5, 1.0, 0.6, 0.4, 0.2 };
	static const float radius0[5] = { 0.2, 0.15, 0.05, 0.025, 0.015 };
	static const float radius1[5] = { 0.15, 0.05, 0.025, 0.015, 0.005 };
	static const unsigned children0[5] = { 2, 2, 2, 3, 3 };
	static const unsigned children1[5] = { 4, 4, 4, 5, 5 };
	float len = (length1[lvl] - length0[lvl])*rnd() + length0[lvl];
	generate_log(msh, root, axis, segs[lvl], len, radius0[lvl], radius1[lvl],
		     vector3f(0,0,0),0,1);
	if (lvl == 4) {
		// generate leaves
		return;
	} else {
		// generate children
		unsigned c = rnd(children1[lvl] - children0[lvl] + 1) + children0[lvl];
		vector3f rootc = root + axis * len;
		for (unsigned i = 0; i < c; ++i) {
			float a = ((i + 0.5f)/c + rnd() * 0.4f / c) * 2*M_PI;
			vector2f d(cos(a), sin(a));
			vector3f axisc = (axis * len + (d * (len * (0.25f + 1.5*rnd()))).xy0()).normal();
			generate_tree(msh, lvl+1, rootc, axisc);
		}
	}
}

#endif
