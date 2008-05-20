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

// I know that this class is very ineffective but it's only a first test so just ignore it atm

#include "rastered_map.h"

rastered_map::rastered_map(const std::string& header_file, const std::string& data_file, vector2l _cache_tl, long int _square_size, unsigned num_levels) :
				min_height(-10800), max_height(8440), square_size(_square_size), cache_tl(_cache_tl)
{
	// open header file
	xml_doc doc(header_file);
	doc.load();

	// read header file
	xml_elem root = doc.child("dftd-map");
	xml_elem elem = root.child("resolution");
	resolution = elem.attri();
	elem = root.child("max");
	max_lat = elem.attri("latitude");
	max_lot = elem.attri("longitude");
	elem = root.child("min");
	min_lat = elem.attri("latitude");
	min_lot = elem.attri("longitude");	
	// open data file
	data_stream.open(data_file.c_str(), std::ios::binary|std::ios::in);
	if (!data_stream.is_open()) throw std::ios::failure("Could not open file: "+data_file);
	
	levels.resize(num_levels);
	load(levels[levels.size()-1]);
		
	long int newres = square_size/resolution;
	for (int i=levels.size()-2; i>=0; i--) {
		newres*=2;
		sample_up(newres, 0.5, 7035463734, levels[i+1], levels[i]);
	}
/*	
	for (int y=0; y<64; y++) {
		for (int x=0; x<64; x++) {
			std::cout << levels[6][y*64+x] << " ";
		}
		std::cout << std::endl;
	}	
*/	
}

rastered_map::~rastered_map() {
	data_stream.close();
}

void rastered_map::load(std::vector<float>& level)
{
	data_stream.clear();

	short signed int buf[(int)((square_size/resolution)*(square_size/resolution))];
	char *c_buf = (char*)buf;
	
	long int file_width = ((max_lot+abs(min_lot))*3600)/resolution;
	long int file_center = (long int)((max_lat*3600/resolution)*file_width+(abs(min_lot)*3600/resolution));
	
	long int start = (long int)(file_center-(cache_tl.y/resolution)*file_width+cache_tl.x/resolution);
	long int end =   start+(file_width*((square_size-1)/resolution)+(square_size/resolution));
/*	
	int lines = 0;
	for (long int i=start; i<end; i+=file_width) 
	{
		lines++;
		data_stream.seekg(i*2);
		data_stream.read(c_buf, (int)(square_size/resolution)*2);
		for (int n=0; n < (int)(square_size/resolution); n++) {
			level.push_back((float)buf[n]);
		}
	}
	
*/
	level.resize(16*16);
	for (int y=0; y<16; y++) {
		for (int x=0; x<16; x++) {
			level[y*16+x] = -5000.0;
		}
	}

	level[7*16+7] = -2500.0;
}

void rastered_map::sample_up(long int newres, float scale, long seed, std::vector<float>& buf_in, std::vector<float>& buf_out)
// fixme: gap at right and bottom
{
	std::vector<float> result((newres+1) * (newres+1));
	
	
	// copy level-1 heights
	for (long int y = 0; y < newres/2; ++y) {
		for (long int x = 0; x < newres/2; ++x) {
			result[y*2*(newres+1)+x*2] = buf_in[y*(newres/2)+x];
		}
	}
	// copy col 0 and row 0 to right and bottom border to avoid a gap of 0-values
	// fixme: real values for gap
	result[(newres+1)*(newres+1)-1] = result[0];
	for (long int x=0; x<newres; x++) {
		result[newres*(newres+1)+x] = result[x];
	}
	for (long int y=0; y<newres+1; y++) {
		result[y*(newres+1)+newres] = result[y*(newres+1)];
	}

	std::cout << "Compute Square..." << std::endl;
	compute_square(result, vector2i(0,0), newres+1, newres+1, 1, (unsigned)(log(newres)/log(2)), scale, seed);
/*
	// first row
	for (long int x=0; x<1; x++) {
		for (long int y=1; y<newres; y+=2) {
				result[y*(newres+1)+x] = (result[(y-1)*(newres+1)+x]+result[(y+1)*(newres+1)+x+1]+result[y*(newres+1)+x+1])/3;
		}
	}
	// last row
	for (long int x=newres; x<newres+1; x++) {
		for (long int y=1; y<newres; y+=2) {
				result[y*(newres+1)+x] = (result[(y-1)*(newres+1)+x]+result[(y+1)*(newres+1)+x+1]+result[y*(newres+1)+x-1])/3;
		}
	}
	
	// first col
	for (long int x=1; x<1; x+=2) {
		for (long int y=0; y<1; y++) {
				result[y*(newres+1)+x] = (result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y+1)*(newres+1)+x])/3;
		}
	}
	// last col
	for (long int x=1; x<1; x+=2) {
		for (long int y=newres; y<newres+1; y++) {
				result[y*(newres+1)+x] = (result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y-1)*(newres+1)+x])/3;
		}
	}
	
	//centers
	for (long int x=1; x<newres; x+=2) {
		for (long int y=1; y<newres; y+=2) {
				result[y*(newres+1)+x] = (result[(y-1)*(newres+1)+x-1]+result[(y-1)*(newres+1)+x+1]+result[(y+1)*(newres+1)+x-1]+result[(y+1)*(newres+1)+x+1])/4;
		}
	}
	
	// horizontal edges
	for (long int x=2; x<newres+1; x+=2) {
		for (long int y=1; y<newres+1; y+=2) {
				result[y*(newres+1)+x] = (result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y+1)*(newres+1)+x]+result[(y-1)*(newres+1)+x])/4;
		}
	}
	
	// vertical edges
	for (long int x=1; x<newres+1; x+=2) {
		for (long int y=2; y<newres+1; y+=2) {
				result[y*(newres+1)+x] = (result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y+1)*(newres+1)+x]+result[(y-1)*(newres+1)+x])/4;
		}
	}		
*/	
	buf_out.resize(newres*newres);
	for (long int y = 0; y < newres; ++y) {
		for (long int x = 0; x < newres; ++x) {
			buf_out[y*newres+x] = result[y*(newres+1)+x];
		}
	}
}

void rastered_map::compute_square(std::vector<float>& v, vector2i top_left, unsigned long overall_res, unsigned long square_res, unsigned int iter, unsigned max_iter, float scale, long seed)
{
	unsigned long tl = top_left.y*overall_res+top_left.x;
	unsigned long te = (unsigned long)(tl + floor(square_res/2));
	unsigned long tr = tl+square_res-1;
	unsigned long le = (unsigned long)(tl+floor(square_res/2)*overall_res);
	unsigned long mid = (unsigned long)(le+floor(square_res/2));
	unsigned long re = le+square_res-1;
	unsigned long bl = (unsigned long)(le+floor(square_res/2)*overall_res);
	unsigned long be = (unsigned long)(floor(square_res/2)*overall_res);
	unsigned long br = bl+square_res-1;
	unsigned long ln = le-square_res;
	unsigned long un = te-(square_res-1)*overall_res;
	unsigned long rn = re+square_res;
	unsigned long bn = be+(square_res-1)*overall_res;
	
	//std::cout << "overall: "<<overall_res << " square: "<<square_res << " " <<top_left.x << "," << top_left.y <<" tl: " << tl << std::endl;

if (iter == max_iter) {
	//center
	v[mid] = (v[tl]+v[tr]+v[bl]+v[br])/4;
	// left edge
	if (top_left.x <= 0) 
		v[le] = (v[tl]+v[bl]+v[mid])/3;
	else v[le] = (v[tl]+v[ln]+v[bl]+v[mid])/4;
	
	// right edge
	if (top_left.x+square_res>=overall_res) // 3 neighbours
		v[re] = (v[tr]+v[mid]+v[br])/3;
	else
		v[re] = (v[tr]+v[mid]+v[br]+v[rn])/4;	
	
	// top edge
	if (top_left.y <= 0) // 3 neighbours
		v[te] = (v[tl]+v[tr]+v[mid])/3;
	else
		v[te] = (v[tl]+v[tr]+v[un]+v[mid])/4;
	
	// bottom edge
	if (top_left.y+square_res>=overall_res) // 3 neighbours
		v[be] = (v[bl]+v[br]+v[mid])/3;
	else
		v[be] = (v[bl]+v[br]+v[mid]+v[bn])/4;	
} else {
	
	v[tl] = v[tl];
	v[te] = v[te];
	v[tr] = v[tr];
	v[le] = v[le];
	v[mid] = v[mid];
	v[re] = v[re];
	v[bl] = v[bl];
	v[be] = v[be];
	v[br] = v[br];
}
	if (iter<max_iter) {
		compute_square(v, top_left, overall_res, (unsigned)(ceil(square_res/2)+1), iter+1, max_iter, scale, seed);
		compute_square(v, top_left+vector2i((unsigned int)(floor(square_res/2)), 0), overall_res, (unsigned)ceil(square_res/2)+1, iter+1, max_iter, scale, seed);
		compute_square(v, top_left+vector2i(0,(unsigned int)(floor(square_res/2))), overall_res, (unsigned)ceil(square_res/2)+1, iter+1, max_iter, scale, seed);
		compute_square(v, top_left+vector2i((unsigned int)(floor(square_res/2)), (unsigned int)(floor(square_res/2))), overall_res, (unsigned)ceil(square_res/2)+1, iter+1, max_iter, scale, seed);
	}
}

float rastered_map::compute_height(int detail, const vector2i& _coord)
{
	if (detail<0) detail=0;
	vector2l coord(_coord.x/SECOND_IN_METERS, _coord.y/SECOND_IN_METERS);

	float level_res = (resolution/pow(2, levels.size()-1-detail));
	long int level_width = square_size/level_res;
	long int pos = (((cache_tl.y/level_res)-(coord.y/level_res))*level_width)+((coord.x/level_res)-(cache_tl.x/level_res));
	return levels[detail][pos];
}



color rastered_map::compute_color(int detail, const vector2i& coord)
{
	if (detail<0) detail=0;
		float h = compute_height(detail, coord);
		vector3f n = compute_normal(detail, coord);
		return (n.z > 0.9) ? (h > -5000 ? color(240, 240, 242) : color(20,75+50,20))
				: color(64+h*0.5+32, 64+h*0.5+32, 64+h*0.5+32);
}

#define IA 16807
#define IM 2147483647
#define IQ 127773
#define IR 2836
#define NTAB 32
#define EPS (1.2E-07)
#define MAX(a,b) (a>b)?a:b
#define MIN(a,b) (a<b)?a:b

double rastered_map::ran1(long *idum)
{
	int j,k;
	static int iv[NTAB],iy=0;
	void nrerror();
	static double NDIV = 1.0/(1.0+(IM-1.0)/NTAB);
	static double RNMX = (1.0-EPS);
	static double AM = (1.0/IM);

	if ((*idum <= 0) || (iy == 0)) {
		*idum = MAX(-*idum,*idum);
                for(j=NTAB+7;j>=0;j--) {
			k = *idum/IQ;
			*idum = IA*(*idum-k*IQ)-IR*k;
			if(*idum < 0) *idum += IM;
			if(j < NTAB) iv[j] = *idum;
		}
		iy = iv[0];
	}
	k = *idum/IQ;
	*idum = IA*(*idum-k*IQ)-IR*k;
	if(*idum<0) *idum += IM;
	j = (int)(iy*NDIV);
	iy = iv[j];
	iv[j] = *idum;
	return MIN(AM*iy,RNMX);
}
#undef IA 
#undef IM 
#undef IQ
#undef IR
#undef NTAB
#undef EPS 
#undef MAX
#undef MIN

int iset=0;
float gset;
double rastered_map::gauss(long *idum)
{
	float fac,rsq,v1,v2;

	if (iset == 0) {
    	do {
        	v1=2.0*ran1(idum)-1.0;
        	v2=2.0*ran1(idum)-1.0;
        	rsq=v1*v1+v2*v2;
    	} while	(rsq >= 1.0 || rsq == 0.0);
    	fac=sqrt(-2.0*log(rsq)/rsq);
    	gset=v1*fac;
    	iset=1;
    	return v2*fac;
	} else {
    	iset=0;
	    return gset;
	}
}

