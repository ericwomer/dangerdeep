
XVisualInfo *(*DFTD_glXChooseVisual)( Display *dpy, int screen, int *attribList );
#define glXChooseVisual( a, b, c ) (*DFTD_glXChooseVisual)( (a), (b), (c) )
GLXContext (*DFTD_glXCreateContext)( Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct );
#define glXCreateContext( a, b, c, d ) (*DFTD_glXCreateContext)( (a), (b), (c), (d) )
Bool (*DFTD_glXMakeCurrent)( Display *dpy, GLXDrawable drawable, GLXContext ctx );
#define glXMakeCurrent( a, b, c ) (*DFTD_glXMakeCurrent)( (a), (b), (c) )
void (*DFTD_glXDestroyContext)( Display *dpy, GLXContext ctx );
#define glXDestroyContext( a, b ) (*DFTD_glXDestroyContext)( (a), (b) )


Display *(*DFTD_XOpenDisplay)(char *display_name);
#define XOpenDisplay( a ) (*DFTD_XOpenDisplay)( (a) )
int (*DFTD_XCloseDisplay)(Display *display);
#define XCloseDisplay( a ) (*DFTD_XCloseDisplay)( (a) )
Window (*DFTD_XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int c_class, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes);
#define XCreateWindow( a, b, c, d, e, f, g, h, i, j, k, l ) (*DFTD_XCreateWindow)( (a), (b), (c), (d), (e), (f), (g), (h), (i), (j), (k), (l) )
Colormap (*DFTD_XCreateColormap)(Display *display, Window w, Visual *visual, int alloc);
#define XCreateColormap( a, b, c, d ) (*DFTD_XCreateColormap)( (a), (b), (c), (d) )
int (*DFTD_XDestroyWindow)(Display *display, Window w);
#define XDestroyWindow( a, b ) (*DFTD_XDestroyWindow)( (a), (b) )
