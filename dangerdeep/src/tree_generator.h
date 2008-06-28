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
oberes ende mit kappe verschließen, <<<<<<<<<<<<<<<<<<<<<<<<
texturkoordinaten generieren, evtl.
auch wind-faktor (für bewegung), der ist am stamm 0 und wird immer größer bis zu den blättern.
^^^<<<<<<<<<<<<<<<<<<<
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
	std::auto_ptr<model> generate() const;
 protected:
	// bend: evtl. gebe besser axis unten und oben an, dazwischen einfach biegen
	vector3f generate_log(model::mesh& msh, model::mesh& mshleaves, unsigned lvl,
			      const vector3f& root, const vector3f& axis, unsigned segs,
			      float length, float radius0, float radius1,
			      float bend_factor = 0.0f, unsigned bend_segs = 1,
			      bool generate_leaves = false) const;
	void generate_tree(model::mesh& msh, model::mesh& mshleaves, unsigned lvl,
			   const vector3f& root, const vector3f& axis) const;
};

std::auto_ptr<model> tree_generator::generate() const
{
	std::auto_ptr<model::mesh> msh(new model::mesh("tree"));
	std::auto_ptr<model::mesh> mshleaves(new model::mesh("leaves"));
	for (int y = -2; y <= 2; ++y) {
		for (int x = -2; x <= 2; ++x) {
			generate_tree(*msh, *mshleaves, 0, vector3f(3*x,3*y,0), vector3f(0,0,1));
		}
	}
	std::auto_ptr<model> mdl(new model());

	model::material_glsl* mat2 = new model::material_glsl("bark", "reliefmapping.vshader", "reliefmapping.fshader");
	mat2->nrtex = 2;
	mat2->texnames[0] = "tex_color";
	mat2->texnames[1] = "tex_normal";

	model::material::map* dmap = new model::material::map();
	model::material::map* bmap = new model::material::map();
	//model::material::map* smap = new model::material::map();
	dmap->set_texture(new texture(get_texture_dir() + "barktest1.png", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	bmap->set_texture(new texture(get_texture_dir() + "treebarktest_normal.png", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	//smap->set_texture(new texture(get_texture_dir() + "treebarktest_specular.png", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	//model::material* mat = new model::material();

	mat2->texmaps[0].reset(dmap);
	mat2->texmaps[1].reset(bmap);
	mat2->compute_texloc();

	glsl_shader_setup& gss = mat2->get_shadersetup();
	gss.use();
	int loc_depth_factor = gss.get_uniform_location("depth_factor");
	gss.set_uniform(loc_depth_factor, float(0.002));
	gss.use_fixed();

	//mat->specularmap.reset(smap);
	//mat->colormap.reset(dmap);
	//mat->normalmap.reset(bmap);
	//msh->mymaterial = mat;
	//mdl->add_material(mat);
	msh->mymaterial = mat2;
	mdl->add_mesh(msh.release());
	mdl->add_material(mat2);

	dmap = new model::material::map();
	dmap->set_texture(new texture(get_texture_dir() + "leaves.png", texture::LINEAR_MIPMAP_LINEAR, texture::CLAMP_TO_EDGE));
	model::material* mat = new model::material();
	mat->specular = color::white();
	mat->colormap.reset(dmap);
	mshleaves->mymaterial = mat;

	mdl->add_material(mat);
	mdl->add_mesh(mshleaves.release());

	mdl->compile();
	mdl->set_layout();

	return mdl;
}

vector3f tree_generator::generate_log(model::mesh& msh, model::mesh& mshleaves, unsigned lvl,
				      const vector3f& root, const vector3f& axis, unsigned segs,
				      float length, float radius0, float radius1,
				      float bend_factor, unsigned bend_segs,
				      bool generate_leaves) const
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
	// generate bend spline
	std::vector<vector3f> cp;
	cp.reserve(4);
	cp.push_back(root);
	vector3f xaxis(1, 0, 0); // fixme: if axis==xaxis?!
	vector3f yaxis = axis.cross(xaxis).normal();
	xaxis = yaxis.cross(axis);
	for (unsigned i = 1; i < 3; ++i) {
		vector3f a = root + axis * (float(i)/3 * length) + xaxis * ((rnd()-0.5)*bend_factor*length) + yaxis * ((rnd()-0.5)*bend_factor*length);
		cp.push_back(a);
	}
	cp.push_back(root + axis * length);
	bsplinet<vector3f> bsp(3, cp);
	// generate vertices
	unsigned vidx = old_v;
	for (unsigned b = 0; b <= bend_segs; ++b) {
		float b_fac = float(b)/bend_segs;
		vector3f p = bsp.value(b_fac);
		//p = root + axis * (b_fac * length);
		float r = radius0 + (radius1 - radius0) * b_fac;
		for (unsigned s = 0; s <= segs; ++s) {
			float s_fac = float(s)/segs;
			float a = 2*M_PI * s_fac;
			vector2f d(cos(a), sin(a));
			msh.vertices[vidx] = p + xaxis * (d.x * r) + yaxis * (d.y * r);
			msh.normals[vidx] = (xaxis * d.x + yaxis * d.y).normal();
			msh.tangentsx[vidx] = (yaxis * d.x - xaxis * d.y).normal();
			msh.texcoords[vidx] = vector2f(s_fac, b_fac*length);
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
	if (rnd(4) == 0 && lvl < 3) {
		// generate side child
		unsigned c = rnd(2) == 0 ? 2 : 1;
		for (unsigned i = 0; i < c; ++i) {
			float a = (i + 0.1f + rnd() * 0.8f) * 2*M_PI / c;
			vector2f d(cos(a), sin(a));
			vector3f axisc = (axis * 1.0f + xaxis * d.x + yaxis * d.y).normal();
			generate_tree(msh, mshleaves, lvl+2, bsp.value(0.3 + 0.4*rnd()), axisc);
		}
	}
	if (generate_leaves) {
		unsigned nrl = unsigned(length / 0.05) * 2;
		const unsigned nr_verts = nrl*4;
		const unsigned nr_indis = nrl*12;
		xaxis.z = 0;
		yaxis.z = 0;
		xaxis.normalize();
		yaxis.normalize();
		unsigned old_v = mshleaves.vertices.size();
		unsigned old_i = mshleaves.indices.size();
		mshleaves.indices_type = model::mesh::pt_triangles;
		mshleaves.vertices.resize(old_v + nr_verts);
		mshleaves.normals.resize(old_v + nr_verts);
		//mshleaves.tangentsx.resize(old_v + nr_verts);
		mshleaves.texcoords.resize(old_v + nr_verts);
		//mshleaves.righthanded.resize(old_v + nr_verts, true);
		mshleaves.indices.resize(old_i + nr_indis);
		unsigned vidx = old_v;
		unsigned iidx = old_i;
		for (unsigned i = 0; i < nrl; ++i) {
			vector3f p = bsp.value(float(i/2)/(nrl/2));
			mshleaves.vertices[vidx] = p;
			mshleaves.normals[vidx] = axis;
			unsigned lt = rnd(4);
			mshleaves.texcoords[vidx] = vector2f((0.5f+lt)*0.25, 0.0f);
			++vidx;
			float x = (i & 1) ? -0.05f : 0.05f;
			float y = (i & 1) ? -0.05f : 0.05f;
			mshleaves.vertices[vidx] = p + xaxis * x + yaxis * y;
			mshleaves.normals[vidx] = axis;
			mshleaves.texcoords[vidx] = vector2f((1.0f+lt)*0.25, 0.5f);
			++vidx;
			mshleaves.vertices[vidx] = p + xaxis * x + yaxis * -y;
			mshleaves.normals[vidx] = axis;
			mshleaves.texcoords[vidx] = vector2f((0.0f+lt)*0.25, 0.5f);
			++vidx;
			mshleaves.vertices[vidx] = p + xaxis * 2*x;
			mshleaves.normals[vidx] = axis;
			mshleaves.texcoords[vidx] = vector2f((0.5f+lt)*0.25, 1.0f);
			++vidx;
			//fixme: use tri-strips for leaves, gives 6 indices per leaf
			//(double sides) plus 2 degen. -> 8 indices per leaf, not 12!
			//plus better vertex post-TL-cache usage
			mshleaves.indices[iidx++] = vidx-4;
			mshleaves.indices[iidx++] = vidx-3;
			mshleaves.indices[iidx++] = vidx-2;
			mshleaves.indices[iidx++] = vidx-2;
			mshleaves.indices[iidx++] = vidx-3;
			mshleaves.indices[iidx++] = vidx-1;
			mshleaves.indices[iidx++] = vidx-4;
			mshleaves.indices[iidx++] = vidx-2;
			mshleaves.indices[iidx++] = vidx-3;
			mshleaves.indices[iidx++] = vidx-3;
			mshleaves.indices[iidx++] = vidx-2;
			mshleaves.indices[iidx++] = vidx-1;
		}
	}
	//return root + axis * length;
	return bsp.value(1.0);
}

void tree_generator::generate_tree(model::mesh& msh, model::mesh& mshleaves, unsigned lvl,
				   const vector3f& root, const vector3f& axis) const
{
	static const unsigned segs[5] = { 16, 12, 8, 6, 3 };
	static const float length0[5] = { 1.0, 0.4, 0.4, 0.3, 0.3 };
	static const float length1[5] = { 1.5, 0.6, 0.6, 0.4, 0.4 };
	static const float radius0[5] = { 0.15, 0.12, 0.05, 0.025, 0.015 };
	static const float radius1[5] = { 0.12, 0.05, 0.025, 0.015, 0.005 };
	static const float anglefac[5] = { 1.5, 2.0, 2.5, 3.0, 3.5 };
	static const unsigned children0[5] = { 2, 2, 3, 4, 4 };
	static const unsigned children1[5] = { 4, 4, 5, 6, 7 };
	static const float bendfac[5] = { 0.3, 0.5, 0.6, 0.7, 0.8 };
	float len = (length1[lvl] - length0[lvl])*rnd() + length0[lvl];
	vector3f rootc = generate_log(msh, mshleaves, lvl,
				      root, axis, segs[lvl], len, radius0[lvl], radius1[lvl],
				      bendfac[lvl], 4, lvl==4);
	if (lvl == 4) {
		return;
	} else {
		// generate children
		vector3f xaxis(1, 0, 0); // fixme: if axis==xaxis?!
		vector3f yaxis = axis.cross(xaxis).normal();
		xaxis = yaxis.cross(axis);
		unsigned c = rnd(children1[lvl] - children0[lvl] + 1) + children0[lvl];
		for (unsigned i = 0; i < c; ++i) {
			float a = (i + 0.1f + rnd() * 0.8f) * 2*M_PI / c;
			vector2f d(cos(a), sin(a));
			d *= len * (0.25f + anglefac[lvl]*rnd());
			vector3f axisc = (axis * len + xaxis * d.x + yaxis * d.y).normal();
			generate_tree(msh, mshleaves, lvl+1, rootc, axisc);
		}
	}
}

#endif
