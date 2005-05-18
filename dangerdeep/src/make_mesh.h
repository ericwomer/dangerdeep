// helper functions to create meshes
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef MAKE_MESH_H
#define MAKE_MESH_H

#include "model.h"

namespace make_mesh {

// helper function, give vertex indices, base index number, normal and direction
void make_quadface(model::mesh& m, unsigned a, unsigned b, unsigned c, unsigned d,
		   unsigned e, float uscal, float vscal,
		   const vector3f& nm, const vector3f& tg, bool out);

// size in x,y,z direction, the facemask controls which faces will be generated
// bits 0-5, -x,+x,-y,+y,-z,+z aka left wall,right wall,bottom wall, top wall,
// floor,ceiling
model::mesh make_cube(float w, float l, float h,
		      float uscal = 1.0f, float vscal = 1.0f, bool out = true,
		      const string& name = "cube", unsigned facemask = 0x3f);
// no geosphere, sorry.
model::mesh make_sphere(float radius, float height,
			unsigned slices = 16, unsigned stacks = 16,
			float uscal = 1.0f, float vscal = 1.0f,
			bool out = true, const string& name = "sphere");
model::mesh make_cylinder(float r, float h, unsigned rsegs = 16,
			  float uscal = 1.0f, float vscal = 1.0f,
			  bool cap = true, bool out = true,
			  const string& name = "cylinder");
// the cone has a crease at its top.
model::mesh make_cone(float r0, float r1, float h, unsigned rsegs = 16,
		      float uscal = 1.0f, float vscal = 1.0f,
		      bool cap = true, bool out = true,
		      const string& name = "cone");
model::mesh make_torus(float outerr, float innerr, unsigned outerrsegs = 32,
		       unsigned innerrsegs = 16,
		       float uscal = 1.0f, float vscal = 1.0f,
		       bool out = true, const string& name = "torus");
// make a cylinder that is screwed around itself
model::mesh make_spiral(float r, float h, float outerr, unsigned rsegs = 16,
			unsigned hsegs = 16, bool out = true,
			const string& name = "spiral");
// generates outerrseggen/outerrsegs of a full circle
// if both values are equal a real torus is made.
model::mesh make_part_of_torus(float outerr, float innerr,
			       unsigned outerrseggen = 8,
			       unsigned outerrsegs = 32,
			       unsigned innerrsegs = 16,
			       float uscal = 1.0f, float vscal = 1.0f,
			       bool cap = true, bool out = true,
			       const string& name = "subtorus");

model::mesh make_heightfield(unsigned resx, unsigned resy, const vector<Uint8>& heights,
			     float xscal, float yscal, float zscal,
			     float xnoise = 0.0f, float ynoise = 0.0f, float znoise = 0.0f,
			     const string& name = "heightfield");

} //namespace

#endif
