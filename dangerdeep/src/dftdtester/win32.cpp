#include <iostream>
#include <set>

#include <windows.h>
#include <GL/gl.h>

#include "tests.h"
#include "win32.h"

char szClassName[] = "dftdtester";

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HGLRC hglrc;
	HDC hdc;

PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
    1,                                // version number 
    PFD_DRAW_TO_WINDOW |              // support window 
    PFD_SUPPORT_OPENGL |              // support OpenGL 
    PFD_DOUBLEBUFFER,                 // double buffered 
    PFD_TYPE_RGBA,                    // RGBA type 
    24,                               // 24-bit color depth 
    0, 0, 0, 0, 0, 0,                 // color bits ignored 
    0,                                // no alpha buffer 
    0,                                // shift bit ignored 
    0,                                // no accumulation buffer 
    0, 0, 0, 0,                       // accum bits ignored 
    32,                               // 32-bit z-buffer     
    0,                                // no stencil buffer 
    0,                                // no auxiliary buffer 
    PFD_MAIN_PLANE,                   // main layer 
    0,                                // reserved 
    0, 0, 0                           // layer masks ignored 
};

	switch (message)                  /* handle the messages */
	{
		case WM_CREATE:
			int  iPixelFormat;
			hdc = GetDC(hwnd);

			iPixelFormat = ChoosePixelFormat(hdc, &pfd);

			SetPixelFormat(hdc, iPixelFormat, &pfd);


			if (hglrc = wglCreateContext( hdc ) )
			{
				wglMakeCurrent(hdc, hglrc);
			}

			return DefWindowProc (hwnd, message, wParam, lParam);
		break;
		case WM_DESTROY:

			if(hglrc = wglGetCurrentContext())
			{
				hdc = wglGetCurrentDC();

				wglMakeCurrent(NULL, NULL);

				ReleaseDC (hwnd, hdc );

				wglDeleteContext(hglrc);
			}

			PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
		break;
		default:                      /* for messages that we don't deal with */
			return DefWindowProc (hwnd, message, wParam, lParam);
	}

	return 0;
}



void win32::args( HINSTANCE a, HINSTANCE b, LPSTR c, int d )
{
	hThisInstance = a;
	hPrevInstance = b;
	lpszArgument = c;
	nFunsterStil = d;
}

int win32::load_ctx()
{
	/* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
	wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
	wincl.cbSize = sizeof (WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;                 /* No menu */
	wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;                      /* structure or the window instance */

	/* Use Windows's default color as the background of the window */
	wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

	/* Register the window class, and if it fails quit the program */
	if (!RegisterClassEx (&wincl))
		return 0;

	/* The class is registered, let's create the program*/
	hwnd = CreateWindowEx (
		0,                   /* Extended possibilites for variation */
		szClassName,         /* Classname */
		"gltest.bin",       /* Title Text */
		WS_OVERLAPPEDWINDOW, /* default window */
		CW_USEDEFAULT,       /* Windows decides the position */
		CW_USEDEFAULT,       /* where the window ends up on the screen */
		544,                 /* The programs width */
		375,                 /* and height in pixels */
		HWND_DESKTOP,        /* The window is a child-window to desktop */
		NULL,                /* No menu */
		hThisInstance,       /* Program Instance handler */
		NULL                 /* No Window Creation data */
		);

	/* Make the window visible on the screen */
//	ShowWindow (hwnd, nFunsterStil);

/*
	// Run the message loop. It will run until GetMessage() returns 0 
	while (GetMessage (&messages, NULL, 0, 0))
	{
		// Translate virtual-key messages into character messages
		TranslateMessage(&messages);
		// Send message to WindowProcedure 
		DispatchMessage(&messages);
	}*/

	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}

int win32::unload_ctx()
{
	system("PAUSE");
	return 0;
}

int win32::loadlibs()
{
	return 0;
}

int win32::unloadlibs()
{
	return 0;
}
