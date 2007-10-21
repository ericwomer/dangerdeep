class win32 : public tests
{
	public:
		void args( HINSTANCE a, HINSTANCE b, LPSTR c, int d );
	protected:
		int load_ctx();
		int unload_ctx();

		int loadlibs();
		int unloadlibs();


	private:
		HWND hwnd;               /* This is the handle for our window */
		MSG messages;            /* Here messages to the application are saved */
		WNDCLASSEX wincl;        /* Data structure for the windowclass */

		HINSTANCE hThisInstance;
		HINSTANCE hPrevInstance;
		LPSTR lpszArgument;
		int nFunsterStil;

};
