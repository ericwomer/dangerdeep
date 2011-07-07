/*
	A quick and simple opengl font library that uses GNU freetype2, written
	and distributed as part of a tutorial for nehe.gamedev.net.
	Sven Olsen, 2003
	
	adapted by Thorsten Jordan
*/


//FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

//Some STL headers
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

//Using the STL exception library increases the
//chances that someone else using our code will corretly
//catch any exceptions that we throw.
#include <stdexcept>
#include <algorithm>


// we use all codes 33-255
#define FIRSTCHAR 33
#define LASTCHAR 255


vector<unsigned> bw;
vector<unsigned> bh;
vector<int> bl;
vector<int> bt;
vector<unsigned> bx;
vector<unsigned> by;
vector<vector<unsigned char> > bd;

struct sorty
{
	unsigned w, h, n;
	sorty(unsigned w_, unsigned h_, unsigned n_) : w(w_), h(h_), n(n_) {}
	bool operator< (const sorty& s) const {
		return (h > s.h) ? true : (h == s.h ? w > s.w : false);
	}
};

struct position
{
	unsigned x, y;
	position(unsigned x_ = 0, unsigned y_ = 0) : x(x_), y(y_) {}
};

std::vector<sorty> sorties;

unsigned ngp2(unsigned x)
{
	unsigned y = 1;
	while (y < x)
		y <<= 1;
	return y;
}

void try_placement(unsigned texw, unsigned& texh, std::vector<position>& positions, unsigned& xwaste, unsigned& ywaste)
{
	positions.resize(sorties.size());
	unsigned sumw = 0, sumh = 0, waste = 0, curh = sorties.front().h, waste2 = 0;
	bool failed = false;
	unsigned x_gap = 0, y_gap = 0;
	unsigned maxh = 2048;
	for (unsigned j = 0; j < unsigned(sorties.size()); ++j) {
		sorty& s = sorties[j];
		positions[j].x = sumw;
		positions[j].y = sumh;
		if (sorties[j].w == 0)
			continue;
		unsigned neww = sumw + x_gap + s.w;
		if (neww > texw) {
			// we could try to place other characters on rest
			// space of that row, but this only saves few wasted
			// pixels, not enough to bring down texture heights
			// significantly.
			// rest on that row (without gap) is: texw-sumw
			waste += (texw - sumw) * curh;
			sumh += y_gap + curh;
			if (sumh > maxh) {
				failed = true;
				break;
			}
			y_gap = 1;
			positions[j].x = 0;
			positions[j].y = sumh;
			sumw = s.w;
			curh = s.h;
		} else {
			sumw = neww;
		}
		x_gap = 1;
	}
	if (!failed) {
		// handle space calculation of last row
		// waste of unfilled row
		waste += (texw - sumw) * curh;
		sumh += y_gap + curh;
		if (sumh > maxh)
			failed = true;
		// waste of vertically unfilled texture
		waste2 = texw * (ngp2(sumh) - sumh);
	}
	if (failed) {
		std::cout << "failed x=" << texw << "\n";
		texh = 0;
		xwaste = ywaste = texw*texw;
	}
	texh = sumh;
	xwaste = waste;
	ywaste = waste2;
}

void make_char ( FT_Face face, unsigned ch, unsigned h)
{

	//The first thing we do is get FreeType to render our character
	//into a bitmap.  This actually requires a couple of FreeType commands:

	//Load the Glyph for our character.
	if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
		throw std::runtime_error("FT_Load_Glyph failed");

	//Move the face's glyph into a Glyph object.
	FT_Glyph glyph;
	if(FT_Get_Glyph( face->glyph, &glyph ))
		throw std::runtime_error("FT_Get_Glyph failed");

	//Convert the glyph to a bitmap.
	FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	//This reference will make accessing the bitmap easier
	FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	//Use our helper function to get the widths of
	//the bitmap data that we will need in order to create
	//our texture.
	int cbw = bitmap.width, cbh = bitmap.rows;
	bw[ch] = cbw;
	bh[ch] = cbh;

	//Allocate memory for the texture data.
	vector<unsigned char>& expanded_data = bd[ch];
	expanded_data.resize(cbw * cbh);

	//Here we fill in the data for the expanded bitmap.
	for(int j=0; j <cbh;j++)
		for(int i=0; i < cbw; i++)
			expanded_data[i+j*cbw] = bitmap.buffer[i + cbw*j];

	bl[ch] = bitmap_glyph->left;
	bt[ch] = bitmap_glyph->top;

	printf("generating char %i (%c) width %i height %i left %i top %i\n",ch,ch,cbw,cbh,bl[ch],bt[ch]);
}



void init(const char * fname, unsigned int h, string outputfilename)
{
	bw.resize(LASTCHAR+1);
	bh.resize(LASTCHAR+1);
	bl.resize(LASTCHAR+1);
	bt.resize(LASTCHAR+1);
	bd.resize(LASTCHAR+1);
	bx.resize(LASTCHAR+1);
	by.resize(LASTCHAR+1);

	//Create and initilize a freetype font library.
	FT_Library library;
	if (FT_Init_FreeType( &library )) 
		throw std::runtime_error("FT_Init_FreeType failed");

	//The object in which Freetype holds information on a given
	//font is called a "face".
	FT_Face face;

	//This is where we load in the font information from the file.
	//Of all the places where the code might die, this is the most likely,
	//as FT_New_Face will die if the font file does not exist or is somehow broken.
	if (FT_New_Face( library, fname, 0, &face )) 
		throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");

	//For some twisted reason, Freetype measures font size
	//in terms of 1/64ths of pixels.  Thus, to make a font
	//h pixels high, we need to request a size of h*64.
	//(h << 6 is just a prettier way of writting h*64)
	FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);

	//This is where we actually create each of the fonts display lists.
	for(unsigned i=FIRSTCHAR;i<LASTCHAR+1;i++)
		make_char(face,i,h);

	//We don't need the face information now that the display
	//lists have been created, so we free the assosiated resources.
	FT_Done_Face(face);

	//Ditto for the library.
	FT_Done_FreeType(library);

	// rearrange data to best fit in a power of two texture
	// try all texture sizes from 2^6 to 2^12 in width
	// and fill in characters sorted by height and then width
	// with one row/colum spacing between them
	// and use arrangement with least space waste
	for(unsigned i=FIRSTCHAR;i<LASTCHAR+1;i++) {
		sorties.push_back(sorty(bw[i], bh[i], i));
	}
	std::sort(sorties.begin(), sorties.end());

	unsigned min_waste = unsigned(-1);
	unsigned min_waste_texw = 0;
	unsigned min_waste_sumh = 0;
	std::vector<position> min_waste_positions;
	for (unsigned texw = 64; texw <= 2048; texw <<= 1) {
		unsigned sumh = 0;
		std::vector<position> positions;
		unsigned xwaste = 0, ywaste = 0;
		try_placement(texw, sumh, positions, xwaste, ywaste);
		unsigned waste = xwaste * 8 + ywaste;
		if (waste < min_waste)	{
			min_waste = waste;
			min_waste_texw = texw;
			min_waste_sumh = sumh;
			min_waste_positions.swap(positions);
			std::cout << "new minimum:\n";
		}
		if (sumh > 0) {
			unsigned texh = ngp2(sumh);
			std::cout << "texture " << texw << "x" << texh << " (h=" << sumh
				  << ") with wasted pixels: " << xwaste << " + " << ywaste
				  << " = " << xwaste + ywaste << "\n";
		}
	}

	// now allocate texture and fill in characters, using min_waste_positions data
	vector<unsigned char> all(min_waste_texw*min_waste_sumh);
	for (unsigned i = 0; i < unsigned(sorties.size()); ++i) {
		unsigned j = sorties[i].n;
		if (sorties[i].w > 0) {
			bx[j] = min_waste_positions[i].x;
			by[j] = min_waste_positions[i].y;
			for (unsigned y=0; y<bh[j]; ++y) {
				for (unsigned x=0; x<bw[j]; ++x) {
					all[min_waste_positions[i].x+x+(min_waste_positions[i].y+y)*min_waste_texw] = bd[j][y*bw[j]+x];
				}
			}
		} else {
			bx[j] = by[j] = 0;
		}
	}

	// save font data
	ofstream osg((outputfilename+".pgm").c_str());
	osg << "P5\n" << min_waste_texw << " " << min_waste_sumh << "\n255\n";
	osg.write((const char*)(&all[0]), all.size());
	ofstream osg2((outputfilename+".metric").c_str());
	osg2 << h << "\n";
	osg2 << FIRSTCHAR << " ";
	osg2 << LASTCHAR << "\n";
	for(unsigned i=FIRSTCHAR;i<LASTCHAR+1;i++) {
		osg2 << bx[i] << " ";
		osg2 << by[i] << " ";
		osg2 << bw[i] << " ";
		osg2 << bh[i] << " ";
		osg2 << bl[i] << " ";
		osg2 << bt[i] << "\n";
	}
}

int main(int argc, char** argv)
{
	if (argc < 4) {
		printf("usage %s ttf_font_filename height outputfilename\n",argv[0]);
		return -1;
	}
	unsigned h = atoi(argv[2]);
	std::cout << "Making font " << argv[1] << "," << argv[3] << " with height " << h << "\n";
	init(argv[1], h, argv[3]);
	return 0;
}
