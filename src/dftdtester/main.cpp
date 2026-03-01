
#include <iostream>
#include <set>

#include "tests.h"

#ifdef WIN32

#include "win32.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil) {
    win32 *handle = new win32();

    handle->args(hThisInstance, hPrevInstance, lpszArgument, nFunsterStil);

    return handle->main();
}

#else

#include "x11.h"
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <dlfcn.h>

int main(int argc, char **argv) {
    x11 *handle = new x11();
    return handle->main();
}

#endif
