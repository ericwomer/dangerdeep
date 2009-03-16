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

#include <SDL.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <list>

#include "../vector2.h"
#include "../morton_bivector.h"
#include "../bitstream.h"
#include "../mymain.cpp"
#include "../bzip.h"
#include "../terrain.h"

#define write_Sint16(val,len,shift,stream); { if(val<0) {stream.write((Uint16)((-val)>>shift), len); stream.write((Uint8)1,1);} else {stream.write((Uint16)(val>>shift),len);stream.write((Uint8)0,1);}}
#define write_Uint8(val,len,stream); {stream.write((Uint8)(val), len); stream.write((Uint8)0,1);}
#define read_bw(target, len, stream); {target = stream.read(len); if(stream.read(1)==1) {target=(-target);}}
#define read_bw_s(target, len, shift, stream); {target = stream.read(len); target<<=shift; if(stream.read(1)==1) {target=(-target);}}

inline void print_tile(morton_bivector<Sint16>& tile) 
{

	for(int y=0; y<tile.size(); y++) {	
		for(int x=0; x<tile.size(); x++) {
			std::cout << tile.at(x,y) << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

inline void load_tile(ifstream& file, morton_bivector<Sint16>& tile, vector2i tl, vector2i map_size, vector2i padded_size)
{
	// from tl to br ...
	
	float buffer = 0.0;
	for(int y=0; y<tile.size(); y++) {	
		for(int x=0; x<tile.size(); x++) {
			
			vector2i pos(tl.x+x, tl.y+y);
			if((padded_size.x-pos.x)>map_size.x || (padded_size.y-pos.y)>map_size.y) tile.at(x,y) = -9999;
			else {
				file.seekg(((pos.y*map_size.x)+pos.x)*sizeof(float));
				file.read((char*)&buffer, sizeof(buffer));
				tile.at(x,y) = (Sint16)buffer;
			}  
		}
	}
}

inline void compute_average(morton_bivector<Sint16>& tile, Sint16& average, long& numbits) {
	
	long double tile_average_arith = 0, tile_average_rms = 0, tile_average_harm = 0;
	long double tile_average_arith_max = 0, tile_average_rms_max = 0, tile_average_harm_max = 0;
	long double tile_average_arith_min = 0, tile_average_rms_min = 0, tile_average_harm_min = 0;

	Sint16 tile_numbits_arith = 0, tile_numbits_rms = 0, tile_numbits_harm = 0, tile_average = 0, tile_numbits = 0;
	
	for(int y=0; y<tile.size(); y++) {	
		for(int x=0; x<tile.size(); x++) {
			tile_average_arith += (long double)tile.at(x, y);
			tile_average_rms += (long double)(tile.at(x, y)*tile.at(x, y));
			tile_average_harm += 1.0/(long double)tile.at(x, y);
		}
	}
	
	tile_average_arith /= (long double)(tile.size()*tile.size());
	tile_average_rms = sqrt(tile_average_rms/(long double)(tile.size()*tile.size()));
	tile_average_harm = ((long double)(tile.size()*tile.size()))/tile_average_harm;
	
	long double distance;
	
	for(int y=0; y<tile.size(); y++) {	
		for(int x=0; x<tile.size(); x++) {
			distance = ((long double)tile.at(x, y))-tile_average_arith;
			if(distance>tile_average_arith_max)tile_average_arith_max=distance;
			if(distance<tile_average_arith_min)tile_average_arith_min=distance;
					
			distance = ((long double)tile.at(x, y))-tile_average_rms;
			if(distance>tile_average_rms_max)tile_average_rms_max=distance;
			if(distance<tile_average_rms_min)tile_average_rms_min=distance;
					
			distance = ((long double)tile.at(x, y))-tile_average_harm;
			if(distance>tile_average_harm_max)tile_average_harm_max=distance;
			if(distance<tile_average_harm_min)tile_average_harm_min=distance;
		}
	}
	
	if((-tile_average_arith_min) > tile_average_arith_max) tile_average_arith_max = -tile_average_arith_min;
	if((-tile_average_rms_min) > tile_average_rms_max) tile_average_rms_max = -tile_average_rms_min;
	if((-tile_average_harm_min) > tile_average_harm_max) tile_average_harm_max = -tile_average_harm_min;
	
	if(tile_average_arith_max != (long double)0.)
		tile_numbits_arith = floor(log2(ceil(tile_average_arith_max))+1.);
	else tile_numbits_arith = 1;
	
	if(tile_average_rms_max != (long double)0.)
		tile_numbits_rms = floor(log2(ceil(tile_average_rms_max))+1.);
	else tile_numbits_arith = 1;
	
	if(tile_average_harm_max != (long double)0.)
		tile_numbits_harm = floor(log2(ceil(tile_average_harm_max))+1.);
	else tile_numbits_arith = 1;
	
	tile_average=tile_average_arith; tile_numbits=tile_numbits_arith;
	if(tile_numbits_rms<tile_numbits) { tile_average=tile_average_rms; tile_numbits=tile_numbits_rms; }
	if(tile_numbits_harm<tile_numbits) { tile_average=tile_average_harm; tile_numbits=tile_numbits_harm; }
	
	average = tile_average;
	numbits = tile_numbits;
	if(numbits < 1) numbits = 1;
}

inline void compare_tiles(morton_bivector<Sint16>& tile1, morton_bivector<Sint16>& tile2, Uint16 max_diff) 
{
	std::cout << "compare: ";

	for(long y=0; y<tile1.size(); y++) {
		for(long x=0; x<tile1.size(); x++) {
			if(std::abs(tile1.at(x,y)-tile2.at(x,y)) > max_diff) {
				std::cerr << std::endl << tile1.at(x,y) << " <> " << tile2.at(x,y) << " at position: " << x << "," << y << std::endl; 
				exit(-1);
			}
		}
	}
	std::cout << "OK" << std::endl;
 
}

inline void read_tile_bit(morton_bivector<Sint16>& tile, const char* filename) 
{
	Sint16 average;
	long numbits;
	long shift;
	
	ifstream file;
	file.open(filename);
	bzip_istream bin(&file);
	ibitstream in(&bin);
	
	read_bw(average, 15, in);
	
	read_bw(numbits, 4, in);
	read_bw(shift, 4, in);
	
	Sint16 *ptr = tile.data_ptr();
	for(long i=0; i<tile.size()*tile.size(); i++) {
		read_bw_s((*ptr), numbits, shift, in);
		ptr++;
	}
	tile += average;
	bin.close();
	file.close();
}

inline void write_tile_bit(morton_bivector<Sint16>& tile, std::ostream& file, long& shift)
{
	bzip_ostream bout(&file);
	
	long numbits = 0;
	Sint16 average = 0;
	
	compute_average(tile, average, numbits);
	std::cout << "numbits: " << numbits << std::endl;
	numbits -= shift;
	tile += -average;

	obitstream out(&bout);
	write_Sint16(average, 15, 0, out);
	
	write_Uint8(numbits, 4, out);
	write_Uint8(shift, 4, out);

	Sint16 *ptr = tile.data_ptr();
	for(long i=0; i<tile.size()*tile.size(); i++) {
		write_Sint16((*ptr), numbits, shift, out);
		ptr++;
	}
	out.last_write();
	bout.close();
}

int mymain(list<string>& args)
{

	std::string infile, outdir;
	long rows = 10800;
	long cols = 21600;
	long sqr_size = 512;
	long shift = 0;
	bool compare = false;
	bool clip = false;
	vector2i clip_tl, clip_br;
	
	for (std::list<std::string>::iterator it = args.begin(); it != args.end(); ++it) {
		if(*it == "--help") {
			std::cout   << "*** Danger from the Deep maptool ***"																<< std::endl
						<< "usage: map_precompute --infile <file> --outdir <dir> [options]\n"									<< std::endl
						<< "options:"																							<< std::endl
						<< "\t--help\t\t\tshow this"																			<< std::endl
						<< "\t--infile <file>\t\tthe input file"																<< std::endl
						<< "\t--outdir <dir>\t\tthe output directory"															<< std::endl
						<< "\t--mapsize X*Y\t\tspecifies map resolution"														<< std::endl
						<< "\t\t\t\tDefault: 21600*10800"																		<< std::endl
						<< "\t--tile_size x\t\tthe size for each tile in coarsest level. needs to be power of 2"				<< std::endl
						<< "\t--shift x\t\tright shifts all height values by x bits"											<< std::endl
						<< "\t--levels x\t\tspecifies the number of detail levels for the map"									<< std::endl
						<< "\t--compare\t\treads in the written tile and compare with the original tile"						<< std::endl
						<< "\t--clip X1,Y1 X2,Y2\tonly computes a clipped region of the map."									<< std::endl
						<< "\t\t\t\tthe first X,Y pair are the top left coords, the second pair are the bottom right coords."	<< std::endl
						<< "\t\t\t\tNOTE: the coordinates have to fit the tile size!"											<< std::endl;
			return 0;
		}
		if(*it == "--mapsize=") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				string::size_type st = it2->find("*");
				if (st != string::npos) {
					cols = atol(it2->substr(0, st).c_str());
					rows = atol(it2->substr(st+1).c_str());
				} else {
					std::cout << "Wrong value for --mapsize" << std::endl;
					return -1;
				}
			}
		}
		if(*it == "--infile") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				infile = *it2;
			}
		}
		if(*it == "--outdir") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				outdir = *it2;
			}
		}
		if(*it == "--tile_size") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				sqr_size = atol((*it2).c_str());
			}
		}
		if(*it == "--shift") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				shift = atol((*it2).c_str());
			}
		}
		if(*it == "--compare") {
			compare = true;
		}
		if(*it == "--clip") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				
				string::size_type st = it2->find(",");
				if (st != string::npos) {
					clip_tl.x = atoi(it2->substr(0, st).c_str());
					clip_tl.y = atoi(it2->substr(st+1).c_str());
				}
				++it2;
				st = it2->find(",");
				if (st != string::npos) {
					clip_br.x = atoi(it2->substr(0, st).c_str());
					clip_br.y = atoi(it2->substr(st+1).c_str());
				}
				clip = true;
			}
		}		
	}
	ifstream instream;
	
	instream.open(infile.c_str());
	if(instream.fail()) {
		std::cerr << "Could not open file " << infile << std::endl;
		return(-1);
	}
	
	if(log2(sqr_size) != (long)(log2(sqr_size))) {
		std::cerr << "tile size is non power of 2" << std::endl;
		return(-1);
	}

	unsigned padded_cols = ceil(cols/(float)sqr_size)*sqr_size;
	unsigned padded_rows = ceil(rows/(float)sqr_size)*sqr_size;

	unsigned sqr_x_start, sqr_y_start, end_x, end_y;
	if(clip) {
		if(((clip_tl.x/sqr_size)*sqr_size) != clip_tl.x) {
			std::cerr << "clip tl.x is not a multiple of tile size" << std::endl;
			return(-1);
		}
		if(((clip_tl.y/sqr_size)*sqr_size) != clip_tl.y) {
			std::cerr << "clip tl.y is not a multiple of tile size" << std::endl;
			return(-1);
		}
		if(((clip_br.x/sqr_size)*sqr_size) != clip_br.x) {
			std::cerr << "clip br.x is not a multiple of tile size" << std::endl;
			return(-1);
		}
		if(((clip_br.y/sqr_size)*sqr_size) != clip_br.y) {
			std::cerr << "clip br.y is not a multiple of tile size" << std::endl;
			return(-1);
		}
		sqr_x_start = clip_tl.x;
		sqr_y_start = clip_tl.y;
		end_x = clip_br.x;
		end_y = clip_br.y;
	} else {
		sqr_x_start = 0;
		sqr_y_start = 0;
		end_x = padded_cols;
		end_y = padded_rows;
	}
	
	std::cout << "start precomputing with:" << std::endl;
	std::cout << "\tinfile: " << infile << std::endl;
	std::cout << "\toutdir: " << outdir << std::endl;
	std::cout << "\tcols: " << padded_cols << std::endl;
	std::cout << "\trows: " << padded_rows << std::endl;
	std::cout << "\ttile_size: " << sqr_size << std::endl;
	std::cout << "\tshift: " << shift << std::endl;
	std::cout << "\tcompare: " << compare << std::endl;
	std::cout << "\tclip: " << clip << std::endl;
	std::cout << "\tclip_tl: " << clip_tl << std::endl;
	std::cout << "\tclip_br: " << clip_br << std::endl;
	std::cout << "\tstart: " << vector2i(sqr_x_start, sqr_y_start) << std::endl;
	std::cout << "\tend: " << vector2i(padded_cols, padded_rows) << std::endl;
	morton_bivector<Sint16> tile;
	
	for (unsigned sqr_y=sqr_y_start; sqr_y<end_y; sqr_y+=sqr_size) {
		for (unsigned sqr_x=sqr_x_start; sqr_x<end_x; sqr_x+=sqr_size) {

			std::ofstream file;
			stringstream filename, textname, normname;
			
			tile.resize(sqr_size);
			
			load_tile(instream, tile, vector2i(sqr_x, sqr_y), vector2i(cols, rows), vector2i(padded_cols, padded_rows));
		
			//bottom left corner...
			filename << outdir;
			filename << padded_rows-sqr_size-sqr_y;
			filename << "_";
			filename << sqr_x;
			filename << ".bz2";
			
			std::cout << filename.str() << std::endl;

			file.open(filename.str().c_str());
			if(file.fail()) {
				std::cerr << "Can't open " << filename.str() << " for output" << std::endl;
				return(-1);
			}
			
			Uint16 max_diff = 0;
			for(int i=0; i<shift; i++) {
				max_diff <<= 1;
				max_diff += 1;
			}

			if(compare) {
				morton_bivector<Sint16> original_tile(tile);
				write_tile_bit(tile, file, shift);
				file.close();
				read_tile_bit(tile, filename.str().c_str());
				compare_tiles(original_tile, tile, max_diff);
			} else write_tile_bit(tile, file, shift);
			file.close();
		}
	}
	std::cout << "complete" << std::endl;
	return 0;
	 

}

#undef write_Uint8
#undef write_Sint16
#undef read_bw
#undef read_bw_s