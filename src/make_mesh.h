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

#ifndef MAKE_MESH_H
#define MAKE_MESH_H

#include "model.h"

namespace make_mesh {

// helper functions
vector3f vec(int vi);
void quadface(model::mesh* m, unsigned bv, float uscal, float vscal, bool out, int ni, int ti);

// size in x,y,z direction
model::mesh* cube(float w, float l, float h,
		      float uscal = 1.0f, float vscal = 1.0f, bool out = true,
		      const std::string& name = "cube");
// no geosphere, sorry.
model::mesh* sphere(float radius, float height,
			unsigned slices = 16, unsigned stacks = 16,
			float uscal = 1.0f, float vscal = 1.0f,
			bool out = true, const std::string& name = "sphere");
model::mesh* cylinder(float r, float h, unsigned rsegs = 16,
			  float uscal = 1.0f, float vscal = 1.0f,
			  bool cap = true, bool out = true,
			  const std::string& name = "cylinder");
// the cone has a crease at its top.
model::mesh* cone(float r0, float r1, float h, unsigned rsegs = 16,
		      float uscal = 1.0f, float vscal = 1.0f,
		      bool cap = true, bool out = true,
		      const std::string& name = "cone");
model::mesh* torus(float outerr, float innerr, unsigned outerrsegs = 32,
		       unsigned innerrsegs = 16,
		       float uscal = 1.0f, float vscal = 1.0f,
		       bool out = true, const std::string& name = "torus");

/*
// make a cylinder that is screwed around itself
model::mesh spiral(float r, float h, float outerr, unsigned rsegs = 16,
			unsigned hsegs = 16, bool out = true,
			const std::string& name = "spiral");
// generates outerrseggen/outerrsegs of a full circle
// if both values are equal a real torus is made.
model::mesh part_of_torus(float outerr, float innerr,
			       unsigned outerrseggen = 8,
			       unsigned outerrsegs = 32,
			       unsigned innerrsegs = 16,
			       float uscal = 1.0f, float vscal = 1.0f,
			       bool cap = true, bool out = true,
			       const std::string& name = "subtorus");
*/

model::mesh* heightfield(unsigned resx, unsigned resy, const std::vector<Uint8>& heights,
			     float xscal, float yscal, float zscal,
			     float xnoise = 0.0f, float ynoise = 0.0f, float znoise = 0.0f,
			     const std::string& name = "heightfield");

} //namespace

#endif
