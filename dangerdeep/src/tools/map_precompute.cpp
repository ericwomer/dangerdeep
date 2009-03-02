/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Splits the ETOPO1 cell-centered float binary file into tiles */

#include "vector2.h"
#include "bivector.h"
#include <SDL.h>
#include <fstream>
#include <sstream>

using namespace std;

void print_usage() {
	cout << "Usage:" << endl;
	cout << "      map_tiles <input file> <output directory> <number of rows> <number of cells> <tile size>" << endl;
	cout << "      <output directory> needs file separator at the end ('/' on *nix systems)" << endl;
}

inline void zorder_next(vector2i &coord)
{
    ulong b = 1;
    do {
        coord.x ^= b;
        b &= ~coord.x;
        coord.y ^= b;
        b &= ~coord.y;
        b <<= 1;
    } while (b);
}

inline void build_morton_sheme(bivector<vector2i>& sheme, vector2i size)
{
	vector2i coord(0, 0), prev_coord(0,0);
	sheme.resize(size);

	while (!(coord == size)) {
		if ((coord.x < size.x) && (coord.y < size.y)) {
			sheme[prev_coord] = coord;
			prev_coord = coord;
		}
		zorder_next(coord);
	}
	sheme[prev_coord] = size;
}

int main (int argc, char** argv)
{

	if (argc != 6) {
		print_usage();
		exit(0);
	}

	ifstream instream(argv[1]);

	long rows = atol(argv[3]);
	long cols = atol(argv[4]);
	long sqr_size = atol(argv[5]);

	float buffer = 0.0;
	int padded_cols = ceil(cols/(float)sqr_size)*sqr_size;
	int padded_rows = ceil(rows/(float)sqr_size)*sqr_size;

	bivector<Sint16> image(vector2i(padded_cols, padded_rows));
	for (int y=0; y < rows; y++) {
		for (int x=0; x < cols; x++) {
			instream.read((char*)&buffer, sizeof(buffer));
			image.at(x,y+(padded_rows-rows)) = (Sint16)buffer;
		}
	}

	bivector<Sint16> matrice(vector2i(sqr_size, sqr_size));
	
	bivector<vector2i> morton_sheme;
	build_morton_sheme(morton_sheme, vector2i(sqr_size, sqr_size));
	
	for (int sqr_y=0; sqr_y<padded_rows; sqr_y+=sqr_size) {
		for (int sqr_x=0; sqr_x<padded_cols; sqr_x+=sqr_size) {
			
			for (int y=0; y<sqr_size; y++ ) {
				for (int x=0; x<sqr_size; x++ ) {
					matrice.at(x,y) = image.at(sqr_x+x, sqr_y+y);
				}
			}

			stringstream filename;

			filename << argv[2];
			filename << padded_rows-sqr_size-sqr_y;
			
			filename << "_";
			filename << sqr_x;

			filename << ".raw";
			
			cout << filename.str() << endl;
			ofstream outstream(filename.str().c_str());
			
			vector2i coord(0,0);
			while (coord.x < sqr_size && coord.y < sqr_size) {
				outstream.write((char*)&matrice[coord], sizeof(Sint16));
				coord = morton_sheme[coord];
			}
			
			outstream.close();
		}
	}
}

