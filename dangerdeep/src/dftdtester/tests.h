#ifndef WIN32
	#ifndef USE_COLOR
		#define USE_COLOR 1
	#endif
#endif

#ifdef USE_COLOR

#define START_ITEM "\033[1;30m"
#define STOP_ITEM "\033[0m"
#define GOOD "\033[0;32m[Good] :-)\033[0m "
#define MED  "\033[0;33m[Warn] :-\\\033[0m "
#define BAD  "\033[0;31m[Erro] :-(\033[0m "

#else

#define START_ITEM ""
#define STOP_ITEM ""
#define GOOD "[Good] :-) "
#define MED  "[Warn] :-\\ "
#define BAD  "[Erro] :-( "

#endif

#define MPT_OUT( a, b ) { ostringstream __temp; __temp << a; return pt_out( __temp.str(), (b) ); }
enum status { sGOOD, sMED, sBAD };

class tests
{
	public:
		int main();
		int do_gl_tests();
		std::set<std::string> error_log;     // set of generate errors
		std::set<std::string> warn_log;     // set of generate warnings
		virtual ~tests() {}

	protected:
		void load_gl_info();

		int do_version_check();
		int do_texunit_check();
		int do_vbo_check();
		int do_fb_check();
		int do_power2_check();
		int do_fshader_check();
		int do_vshader_check();
		int do_shaderobj_check();

		int pt_out( std::string message, enum status );

		void *opengl;
		void *xlib;

		const char *c_version;
		const char *c_extensions;
		std::string version;
		std::string extensions;

		std::set<std::string> supported_extensions;     // memory supported OpenGL extensions

		bool extension_supported(const std::string& s);

		virtual int loadlibs();
		virtual int unloadlibs();

		virtual int load_ctx() { return 0; }
		virtual int unload_ctx() { return 0; }
};


