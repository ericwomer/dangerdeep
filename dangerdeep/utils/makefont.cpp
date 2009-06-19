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
using namespace std;

//Using the STL exception library increases the
//chances that someone else using our code will corretly
//catch any exceptions that we throw.
#include <stdexcept>


// we use all codes 33-255
#define FIRSTCHAR 33
#define LASTCHAR 255


int maxw = 0, maxh = 0, sumw = 0;
vector<unsigned> bw;
vector<unsigned> bh;
vector<int> bl;
vector<int> bt;
vector<vector<unsigned char> > bd;

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
	if (cbw > maxw) maxw = cbw;
	if (cbh > maxh) maxh = cbh;
	sumw += cbw;

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
	
	printf("maximum width %i height %i, sum of width %i\n",maxw,maxh,sumw);

	// save font data
	vector<unsigned char> all(sumw*maxh);
	unsigned curw = 0;
	for(unsigned i=FIRSTCHAR;i<LASTCHAR+1;i++) {
		for (unsigned y=0; y<bh[i]; ++y) {
			for (unsigned x=0; x<bw[i]; ++x) {
				all[curw+x+y*sumw] = bd[i][y*bw[i]+x];
			}
		}
		curw += bw[i];
	}
	ofstream osg((outputfilename+".pgm").c_str());
	osg << "P5\n" << sumw << " " << maxh << "\n255\n";
	osg.write((const char*)(&all[0]), all.size());
	ofstream osg2((outputfilename+".metric").c_str());
	osg2 << h << "\n";
	osg2 << FIRSTCHAR << " ";
	osg2 << LASTCHAR << "\n";
	for(unsigned i=FIRSTCHAR;i<LASTCHAR+1;i++) {
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
	init(argv[1], h, argv[3]);
	return 0;
}
