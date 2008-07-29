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

#include "xml.h"


#include "rastered_map.h"

rastered_map::rastered_map(const std::string& header_file, const std::string& data_file, unsigned _num_levels) :
num_levels(_num_levels), pn(64, 2, 16), pn2(64, 4, num_levels - 4, true)
{
    extrah.resize(64 * 64);
    for (unsigned y = 0; y < 64; ++y)
        for (unsigned x = 0; x < 64; ++x)
            extrah[64 * y + x] = rnd() - 0.5; //extrah2[64*y+x]/256.0-0.5;
    const char* texnames[8] = {
        "tex_grass.jpg",
        "tex_grass2.jpg",
        "tex_grass3.jpg",
        "tex_grass4.jpg",
        "tex_grass5.jpg",
        "tex_mud.jpg",
        "tex_stone.jpg",
        "tex_sand.jpg"
    };
    for (unsigned i = 0; i < 8; ++i) {
        sdl_image tmp(get_texture_dir() + texnames[i]);
        unsigned bpp = 0;
        ct[i] = tmp.get_plain_data(cw, ch, bpp);
        if (bpp != 3) throw error("color bpp != 3");
    }

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
    max_height = elem.attri("height");
    elem = root.child("min");
    min_lat = elem.attri("latitude");
    min_lot = elem.attri("longitude");
    min_height = elem.attri("height");
    elem = root.child("byte-order");
    byteorder = elem.attrb("value");
    
    // open data file
    data_stream.open(data_file.c_str(), std::ios::binary | std::ios::in);
    if (!data_stream.is_open()) throw std::ios::failure("Could not open file: " + data_file);

}

rastered_map::~rastered_map()
{
    data_stream.close();
}

void rastered_map::compute_heights(int detail, const vector2i& coord_bl, const vector2i& coord_sz,
                                   float* dest, unsigned stride, unsigned line_stride, bool noise)
{
    float level_res = (resolution / pow(2, num_levels - 1 - detail));

    float scale = 1.0;
    if (!stride) stride = 1;
    if (!line_stride) line_stride = coord_sz.x * stride;

    if (detail >= 0) {
        if (detail < (num_levels - 1)) {
            // upsample from the next coarser level
            bivector<float> lower(vector2i((coord_sz.x >> 1) + 1, (coord_sz.y >> 1) + 1));
            compute_heights(detail + 1, vector2i((coord_bl.x >> 1) + 1, (coord_bl.y >> 1) + 1), vector2i((coord_sz.x >> 1) + 1, (coord_sz.y >> 1) + 1), lower.data_ptr(), 0, 0, false);
            bivector<float> heights = lower.upsampled(false);

            for (int y = 0; y < coord_sz.y; ++y) {
                float* dest2 = dest;
                for (int x = 0; x < coord_sz.x; ++x) {
                    vector2i coord = coord_bl + vector2i(x, y);

                    *dest2 = heights.at(x, y);
                    if (noise) *dest2 += pn2.value(coord.x * int(1 << detail), coord.y * int(1 << detail), num_levels - detail) * scale;
                    dest2 += stride;
                }
                dest += line_stride;
            }

        } else if (detail == (num_levels - 1)) {
            // coarsest level - read from file
            data_stream.clear();
            Sint16 buf = 0;
            char *c_buf = (char*) & buf;

            long int file_width = (max_lot + abs(min_lot)) / level_res;
            long int file_center = (long int) ((max_lat / level_res) * file_width + (abs(min_lot) / level_res));

            for (int y = 0; y < coord_sz.y; ++y) {
                float* dest2 = dest;
                for (int x = 0; x < coord_sz.x; ++x) {
                    vector2i coord = coord_bl + vector2i(x, y);
                    vector2f coord_f = vector2f((coord.x / SECOND_IN_METERS) / level_res, (coord.y / SECOND_IN_METERS) / level_res);
                    (coord_f.x < 0)?coord_f.x = floor(coord_f.x):coord_f.x = ceil(coord_f.x);
                    (coord_f.y < 0)?coord_f.y = floor(coord_f.y):coord_f.y = ceil(coord_f.y);


                    data_stream.seekg((file_center - coord_f.y * file_width + coord_f.x) * 2);
                    data_stream.read(c_buf, 2);
                    
                    // if the architectures byte order dont fits the data files byte order we need to swap lsb and msb
                    if ((is_bigendian() && !byteorder) || (!is_bigendian() && byteorder)) {
                        unsigned char c1, c2;
                        c1 = buf&255;
                        c2 = (buf>>8)&255;
                        buf = (c1<<8)+c2;
                    }
                    
                    *dest2 = (float) buf;
                    if (noise) *dest2 += pn2.value(coord.x * int(1 << detail), coord.y * int(1 << detail), num_levels - detail) * scale;
                    dest2 += stride;
                }
                dest += line_stride;
            }
        }

    } else {
        // we need one value more to correctly upsample it
        bivector<float> d0h(vector2i((coord_sz.x >> -detail) + 1, (coord_sz.y >> -detail) + 1));
        compute_heights(0, vector2i(coord_bl.x >> -detail, coord_bl.y >> -detail),
                        d0h.size(), d0h.data_ptr());
        for (int z = detail; z < 0; ++z) {
            bivector<float> tmp = d0h.upsampled(false);
            tmp.swap(d0h);
        }
        for (int y = 0; y < coord_sz.y; ++y) {
            float* dest2 = dest;
            for (int x = 0; x < coord_sz.x; ++x) {
                vector2i coord = coord_bl + vector2i(x, y);
                float baseh = d0h[vector2i(x, y)];
                baseh += extrah[(coord.y & 63)*64 + (coord.x & 63)]*0.25;
                *dest2 = baseh;
                dest2 += stride;
            }
            dest += line_stride;
        }
    }

}

void rastered_map::compute_colors(int detail, const vector2i& coord_bl, const vector2i& coord_sz, Uint8* dest)
{
    for (int y = 0; y < coord_sz.y; ++y) {
        for (int x = 0; x < coord_sz.x; ++x) {
            vector2i coord = coord_bl + vector2i(x, y);
            color c;
            if (detail >= -2) {
                unsigned xc = coord.x << (detail + 2);
                unsigned yc = coord.y << (detail + 2);
                unsigned xc2, yc2;
                if (detail >= 0) {
                    xc2 = coord.x << detail;
                    yc2 = coord.y << detail;
                } else {
                    xc2 = coord.x >> -detail;
                    yc2 = coord.y >> -detail;
                }
                float z = -4500;
                if (detail <= 6) {
                    float h = pn.value(xc2, yc2, 6) / 255.0f;
                    z = -4500 + h * h * h * 0.5 * 256;
                }
                float zif = (z + 130) * 4 * 8 / 256;
                if (zif < 0.0) zif = 0.0;
                if (zif >= 7.0) zif = 6.999;
                unsigned zi = unsigned(zif);
                zif = myfrac(zif);
                //if (zi <= 4) zi = ((xc/256)*(xc/256)*3+(yc/256)*(yc/256)*(yc/256)*2+(xc/256)*(yc/256)*7)%5;
                unsigned i = ((yc & (ch - 1)) * cw + (xc & (cw - 1)));
                float zif2 = 1.0 - zif;
                c = color(uint8_t(ct[zi][3 * i] * zif2 + ct[zi + 1][3 * i] * zif),
                          uint8_t(ct[zi][3 * i + 1] * zif2 + ct[zi + 1][3 * i + 1] * zif),
                          uint8_t(ct[zi][3 * i + 2] * zif2 + ct[zi + 1][3 * i + 2] * zif));
            } else {
                unsigned xc = coord.x >> (-(detail + 2));
                unsigned yc = coord.y >> (-(detail + 2));
                float z = -130;
                if (detail <= 6) {
                    float h = pn.value(xc, yc, 6) / 255.0f;
                    z = -4500 + h * h * h * 0.5 * 256;
                }
                float zif = (z + 130) * 4 * 8 / 256;
                if (zif < 0.0) zif = 0.0;
                if (zif >= 7.0) zif = 6.999;
                unsigned zi = unsigned(zif);
                zif = myfrac(zif);
                //if (zi <= 4) zi = ((xc/256)*(xc/256)*3+(yc/256)*(yc/256)*(yc/256)*2+(xc/256)*(yc/256)*7)%5;
                unsigned i = ((yc & (ch - 1)) * cw + (xc & (cw - 1)));
                float zif2 = 1.0 - zif;
                c = color(uint8_t(ct[zi][3 * i] * zif2 + ct[zi + 1][3 * i] * zif),
                          uint8_t(ct[zi][3 * i + 1] * zif2 + ct[zi + 1][3 * i + 1] * zif),
                          uint8_t(ct[zi][3 * i + 2] * zif2 + ct[zi + 1][3 * i + 2] * zif));
            }
            dest[0] = c.r;
            dest[1] = c.g;
            dest[2] = c.b;
            dest += 3;
        }
    }
}
