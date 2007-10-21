
class x11 : public tests
{
	protected:
		int load_ctx();
		int unload_ctx();

		int loadlibs();
		int unloadlibs();

	private:
		Display *disp;
		Window root;
		XVisualInfo *xinfo;
		Window win;
		GLXContext ctx;
};
