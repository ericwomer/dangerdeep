
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

// main program
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <glu.h>
#include <SDL.h>
#include <SDL_net.h>

#include "system.h"
#include "texture.h"
#include "thread.h"
#include "faulthandler.h"
#include "shader.h"
#include "fpsmeasure.h"
#include "log.h"
#include "datadirs.h"
#include "mymain.cpp"


// special define needed to compile lavc headers.
#ifndef INT64_C
#define INT64_C(c)    int64_t(c ## LL)
#endif
extern "C" {
/* prevent ffmpeg warnings */
#define attribute_deprecated
#include <libavformat/avformat.h>
}

class videoplay : public thread
{
public:
	struct framebuffer
	{
		std::vector<Uint8> y, uv;
		unsigned width, height;
		bool empty;
		float aspect_ratio;
		double time_stamp;

		framebuffer() : width(0), height(0), empty(true), aspect_ratio(0.0f), time_stamp(-1.0) {}
	};

	videoplay(const std::string& filename, unsigned queue_len = 8);
	void display_loop(double delta_time);
	bool video_finished();
	void request_abort();

protected:
	~videoplay();
	void loop();
	void display(framebuffer& fb);
	static int call_get_buffer(struct AVCodecContext *c, AVFrame *pic);
	static void call_release_buffer(struct AVCodecContext *c, AVFrame *pic);

	std::vector<framebuffer> fb_queue;
	std::list<unsigned> decoded_pictures, free_buffers;
	mutex queue_mtx;
	condvar buffer_available;
	double playback_starttime;
	bool playback_started;
	double current_playtime;
	double real_time;

	int vstr_idx;
	struct AVFormatContext* ictx;
	struct AVInputFormat* ifmt;
	struct AVPacket ipkt;
	struct AVCodec* codec;
	struct AVFrame* picture;
	unsigned frame_nr;
	bool eof;
	double next_frame_pts;

	texture::ptr tex_y, tex_uv;
	glsl_shader_setup myshader;
	unsigned loc_tex_y, loc_tex_uv;
};


videoplay::videoplay(const std::string& filename, unsigned queue_len)
	: thread("videoplay"),
	  fb_queue(queue_len),
	  playback_starttime(-1.0),
	  playback_started(false),
	  current_playtime(0.0),
	  real_time(0.0),
	  vstr_idx(-1),
	  ictx(0),
	  ifmt(0),
	  codec(0),
	  picture(0),
	  frame_nr(0),
	  eof(false),
	  next_frame_pts(-1.0),
	  myshader(get_shader_dir() + "videoplay.vshader",
		   get_shader_dir() + "videoplay.fshader")
{
	for (unsigned i = 0; i < queue_len; ++i)
		free_buffers.push_back(i);

	myshader.use();
	loc_tex_y = myshader.get_uniform_location("tex_y");
	loc_tex_uv = myshader.get_uniform_location("tex_uv");

	// open file
	avcodec_init();
	av_register_all();
	av_log_set_level(AV_LOG_ERROR);
	memset(&ipkt, 0, sizeof(AVPacket));
	if (av_open_input_file(&ictx, filename.c_str(), ifmt, 0, 0) < 0)
		throw error(std::string("error opening ") + filename);
	try {
		int res = av_find_stream_info(ictx);
		if (res < 0)
			throw error(std::string("error finding stream info in ") + filename);

		AVCodecContext* context = 0;
		for (unsigned i = 0; i < unsigned(ictx->nb_streams); i++) {
			AVStream* str = ictx->streams[i];
			switch(str->codec->codec_type) {
			case CODEC_TYPE_VIDEO:
				vstr_idx = int(i);
				context = str->codec;
				break;
			default:
				break;
			}
		}

		if (vstr_idx < 0)
			throw error("no video streams");

		picture = avcodec_alloc_frame();
		codec = avcodec_find_decoder(context->codec_id);

		const unsigned thread_count = 1; // fixme
		if (thread_count > 1) {
			if (avcodec_thread_init(context, thread_count) < 0)
				throw error("avcodec_thread_init() failed");
		}

		if (avcodec_open(context, codec) < 0) {
			// cleanup threads (avcodec_close does it else)
			if (context->thread_opaque) avcodec_thread_free(context);
			throw error("avcodec_open() failed");
		}

		context->opaque = this;
		context->get_buffer = call_get_buffer;
		context->release_buffer = call_release_buffer;

	} catch (...) {
		if (picture) av_freep(&picture);
		av_close_input_file(ictx);
	}
}

videoplay::~videoplay()
{
	avcodec_close(ictx->streams[vstr_idx]->codec);	// also calls avcodec_thread_free()
	av_freep(&picture);
	av_close_input_file(ictx);
	av_free_packet(&ipkt);
}

void videoplay::loop()
{
	// read coded data for one frame
	do {
		av_free_packet(&ipkt);
		int ret = av_read_frame(ictx, &ipkt);
		if (ret < 0) {
			if (url_ferror(ictx->pb) == 0) {
				eof = true;
				break;

			}
			// should never happen... error reading file, try again next frame
			return;
		}
	} while (ipkt.stream_index != vstr_idx);

	double stamp = double((ipkt.pts == AV_NOPTS_VALUE) ? ipkt.dts : ipkt.pts) *
		ictx->streams[vstr_idx]->time_base.num / ictx->streams[vstr_idx]->time_base.den;

	// decode frame
	int got_picture = 0;
	avcodec_get_frame_defaults(picture);
	AVCodecContext* ctx = ictx->streams[vstr_idx]->codec;
	next_frame_pts = stamp;
	if (eof)
		avcodec_decode_video(ctx, picture, &got_picture, 0, 0);
	else
		avcodec_decode_video(ctx, picture, &got_picture, ipkt.data, ipkt.size);
	if (!got_picture) {
		// no more data in input and decoder has finished: stop thread
		if (eof)
			thread::request_abort();
		return;
	}
	++frame_nr;
	// do this only when pts reordering is needed (always except for AVIs and MP4s)
	// or always use DTS, that is always linear
	// can we request that modus from context->b_frame
	//log_debug(ctx->max_b_frames<<","<<ctx->has_b_frames);
	// with this tricks it works mostly
	if (ctx->has_b_frames == 1)
		stamp = *(double*)picture->opaque;

	// wait for free buffer
	int bufnr = -1;
	{
		mutex_locker ml(queue_mtx);
		while (free_buffers.empty()) {
			if (abort_requested())
				return;
			buffer_available.wait(queue_mtx);
		}
		bufnr = free_buffers.front();
		free_buffers.pop_front();
	}

	// reinsert bufnr on error, use try/catch-all ...
	framebuffer& fb = fb_queue[bufnr];
	const uint8_t* ybuf = picture->data[0];
	const uint8_t* ubuf = picture->data[1];
	const uint8_t* vbuf = picture->data[2];
	unsigned w = ctx->width, h = ctx->height;
	if (w * h > fb.y.size()) {
		fb.y.resize(w * h);
		fb.uv.resize(w * h / 2);
	}
	for (unsigned y = 0; y < h; ++y) {
		memcpy(&fb.y[y * w], ybuf + y * picture->linesize[0], w);
	}
	for (unsigned y = 0; y < h/2; ++y) {
		//fixme: optimize
		for (unsigned x = 0; x < w/2; ++x) {
			fb.uv[y * w + 2*x + 0] = *(ubuf + y * picture->linesize[1] + x);
			fb.uv[y * w + 2*x + 1] = *(vbuf + y * picture->linesize[2] + x);
		}
	}
	fb.width = w;
	fb.height = h;
	fb.empty = false;
	fb.aspect_ratio = double(ctx->sample_aspect_ratio.num * w) / (ctx->sample_aspect_ratio.den * h);
	fb.time_stamp = stamp;

	// report as ready
	{
		mutex_locker ml(queue_mtx);
		decoded_pictures.push_back(bufnr);
	}
}

void videoplay::display_loop(double delta_time)
{
	real_time += delta_time;

	// check if there is a picture to display
	int bufnr = -1;
	unsigned nr_full_buffers = 0;
	{
		mutex_locker ml(queue_mtx);
		if (!decoded_pictures.empty()) {
			nr_full_buffers = decoded_pictures.size();
			bufnr = decoded_pictures.front();
			if (!playback_started && nr_full_buffers == fb_queue.size()) {
				playback_starttime = fb_queue[bufnr].time_stamp;
				playback_started = true;
			}
			if (playback_started &&
			    fb_queue[bufnr].time_stamp <= (real_time + playback_starttime))
				decoded_pictures.pop_front();
			else
				bufnr = -1;
		}
	}
	if (bufnr < 0)
		return;

	// display the buffer
	current_playtime = fb_queue[bufnr].time_stamp;
	display(fb_queue[bufnr]);

	// report as ready/free
	{
		mutex_locker ml(queue_mtx);
		free_buffers.push_back(bufnr);
		buffer_available.signal();
	}
}

void videoplay::request_abort()
{
	thread::request_abort();
	mutex_locker ml(queue_mtx);
	buffer_available.signal();
}

bool videoplay::video_finished()
{
	bool result = eof;
	if (!result) {
		mutex_locker ml(queue_mtx);
		result = decoded_pictures.empty();
	}
	return result;
}

void videoplay::display(framebuffer& fb)
{
	if (tex_y.get() && (tex_y->get_width() != fb.width || tex_y->get_height() != fb.height)) {
		tex_y.reset();
		tex_uv.reset();
	}
	if (!tex_y.get()) {
		tex_y.reset(new texture(fb.width, fb.height, GL_LUMINANCE, texture::LINEAR, texture::CLAMP_TO_EDGE));
		// no UV filtering, so use nearest! - we could place UV in planes, not interleaved...
		tex_uv.reset(new texture(fb.width/2, fb.height/2, GL_LUMINANCE_ALPHA, texture::NEAREST, texture::CLAMP_TO_EDGE));
	}
	tex_y->sub_image(0, 0, fb.width, fb.height, fb.y, GL_LUMINANCE);
	tex_uv->sub_image(0, 0, fb.width/2, fb.height/2, fb.uv, GL_LUMINANCE_ALPHA);
	myshader.use();
	myshader.set_gl_texture(*tex_y, loc_tex_y, 0);
	myshader.set_gl_texture(*tex_uv, loc_tex_uv, 1);
	glActiveTexture(GL_TEXTURE0);
	// we assume square pixels for display
	const unsigned sw = sys().get_res_x_2d(), sh = sys().get_res_y_2d();
	double display_aspect_ratio = double(sw) / sh;
	unsigned x, y, w, h;
	if (display_aspect_ratio >= fb.aspect_ratio) {
		// screen is wider than picture
		h = sh;
		y = 0;
		w = unsigned(sw * fb.aspect_ratio / display_aspect_ratio);
		x = (sw - w) / 2;
	} else {
		// picture is wider than screen
		w = sw;
		x = 0;
		h = unsigned(sh * display_aspect_ratio / fb.aspect_ratio);
		y = (sh - h) / 2;
	}
	tex_y->draw(x, y, w, h);
}

int videoplay::call_get_buffer(struct AVCodecContext *c, AVFrame *pic)
{
	int ret = avcodec_default_get_buffer(c, pic);
	double* pts = (double*)av_malloc(sizeof(double));
	*pts = ((videoplay*)(c->opaque))->next_frame_pts;
	pic->opaque = pts;
	return ret;
}


void videoplay::call_release_buffer(struct AVCodecContext *c, AVFrame *pic)
{
	if (pic) av_freep(&pic->opaque);
	avcodec_default_release_buffer(c, pic);
}

// ------------------------------------------- code ---------------------------------------

int mymain(list<string>& args)
{
    // report critical errors (on Unix/Posix systems)
    install_segfault_handler();

    // command line argument parsing
    int res_x = 1024, res_y;
    bool fullscreen = true;

    // parse commandline
    std::string filename = "";
    for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
        if (*it == "--help") {
            cout << "*** Danger from the Deep ***\nusage:\n--help\t\tshow this\n"
                    << "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
                    << "--nofullscreen\tdon't use fullscreen\n"
                    << "--debug\t\tdebug mode: no fullscreen, resolution 800\n"
#if !(defined (WIN32) || (defined (__APPLE__) && defined (__MACH__)))
                    << "--vsync\tsync to vertical retrace signal (for nvidia cards)\n"
#endif
                    << "--nosound\tdon't use sound\n";
            return 0;
        } else if (*it == "--nofullscreen") {
            fullscreen = false;
        } else if (*it == "--debug") {
            fullscreen = false;
            res_x = 800;
#if !(defined (WIN32) || (defined (__APPLE__) && defined (__MACH__)))
        } else if (*it == "--vsync") {
            if (putenv((char*) "__GL_SYNC_TO_VBLANK=1") < 0)
                cout << "ERROR: vsync setting failed.\n";
            //maxfps = 0;
#endif
        } else if (*it == "--res") {
            list<string>::iterator it2 = it;
            ++it2;
            if (it2 != args.end()) {
                int r = atoi(it2->c_str());
                if (r == 512 || r == 640 || r == 800 || r == 1024 || r == 1280)
                    res_x = r;
                ++it;
            }
	} else if (*it == "--consolelog") {
		log::copy_output_to_console = true;
        } else {
		filename = *it;
	}
    }

    // fixme: also allow 1280x1024, set up gl viewport for 4:3 display
    // with black borders at top/bottom (height 2*32pixels)
    res_y = res_x * 3 / 4;
    // weather conditions and earth curvature allow 30km sight at maximum.
    system::create_instance(new class system(1.0, 30000.0 + 500.0, res_x, res_y, fullscreen));
    sys().set_res_2d(1024, 768);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    sys().gl_perspective_fovx(70, 4.0 / 3.0, 1.0, 30000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //	sys().set_max_fps(60);

    log_info("Danger from the Deep");

    fpsmeasure fpsm(1.0f);
    bool paused = false;
    bool quit = false;
    thread::auto_ptr<videoplay> vpl(new videoplay(filename));
    vpl->start();
    unsigned tm = sys().millisec();
    while (!quit) {
        list<SDL_Event> events = sys().poll_event_queue();
        for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
            if (it->type == SDL_KEYDOWN) {
                switch ((*it).key.keysym.sym) {
                    case SDLK_ESCAPE:
			vpl.reset();
                        quit = true;
                        break;
                    default:
                        break;
                }
            }
            if (it->type == SDL_MOUSEBUTTONDOWN) {
		    paused = !paused;
		    tm = sys().millisec();
            }
        }
	if (quit)
		break;

        //glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);

        glColor4f(1, 1, 1, 1);

        sys().prepare_2d_drawing();

	// render...
	if (!paused) {
		unsigned tm2 = sys().millisec();
		vpl->display_loop((tm2 - tm)/1000.0);
		tm = tm2;
	}

        // record fps
        /*float fps = */ fpsm.account_frame();

        sys().unprepare_2d_drawing();

        sys().swap_buffers();
    }
    vpl.reset();

    system::destroy_instance();

    return 0;
}
