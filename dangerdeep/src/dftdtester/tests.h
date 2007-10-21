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



class tests
{
	public:
		int main();

	protected:
		int do_gl_tests();
		void *opengl;
		void *xlib;

		std::set<std::string> supported_extensions;     // memory supported OpenGL extensions
		bool extension_supported(const std::string& s);

		virtual int loadlibs();
		virtual int unloadlibs();

		virtual int load_ctx() { return 0; }
		virtual int unload_ctx() { return 0; }
};


