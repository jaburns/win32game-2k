//----------------------------------------------------------------------------------------------------------------------
// Forked from the iq/rgba 1k OpenGL intro system at https://www.iquilezles.org/code/isystem1k4k/isystem1k4k.htm
//----------------------------------------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <GL/gl.h>
#include "fp.h"
#include <math.h>
#include "glext.h"

#define XRES 1280
#define YRES  720

#include "shader.h"

//extern "C" int _fltused = 0;

const static PIXELFORMATDESCRIPTOR pfd = { 0, 0, PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, 0, 0, 0, 0, 0, 0, 0, 0,
    8, // alpha channel bits
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#ifdef _DEBUG

const static char *funcNames[] = {
    "glCreateProgram",
    "glCreateShader",
    "glShaderSource",
    "glCompileShader",
    "glAttachShader",
    "glLinkProgram",
    "glUseProgram",
    "glGetShaderInfoLog"
};

static void *funcPtrs[8];

#define oglCreateProgram    ((PFNGLCREATEPROGRAMPROC)funcPtrs[0])
#define oglCreateShader     ((PFNGLCREATESHADERPROC)funcPtrs[1])
#define oglShaderSource     ((PFNGLSHADERSOURCEPROC)funcPtrs[2])
#define oglCompileShader    ((PFNGLCOMPILESHADERPROC)funcPtrs[3])
#define oglAttachShader     ((PFNGLATTACHSHADERPROC)funcPtrs[4])
#define oglLinkProgram      ((PFNGLLINKPROGRAMPROC)funcPtrs[5])
#define oglUseProgram       ((PFNGLUSEPROGRAMPROC)funcPtrs[6])
#define oglGetShaderInfoLog ((PFNGLGETSHADERINFOLOGPROC)funcPtrs[7])

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{ 
    RECT rec = { 0, 0, XRES, YRES };
    WNDCLASS wc;

    ZeroMemory( &wc, sizeof(WNDCLASS) );
    wc.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc   = DefWindowProc;
    wc.hInstance     = hPrevInstance;
    wc.lpszClassName = "jb";
    RegisterClass( &wc );

    const int dws = WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
    AdjustWindowRect( &rec, dws, 0 );
    HWND hWnd = CreateWindowEx( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wc.lpszClassName, "win32game-2k", dws,
                               (GetSystemMetrics(SM_CXSCREEN)-rec.right+rec.left)>>1,
                               (GetSystemMetrics(SM_CYSCREEN)-rec.bottom+rec.top)>>1,
                               rec.right-rec.left, rec.bottom-rec.top, 0, 0, hPrevInstance, 0 );
    HDC hDC = GetDC( hWnd );

#else

void entrypoint( void )
{ 
    DEVMODE screenSettings = {{0},0,0,156,0,0x001c0000,{0},0,0,0,0,0,{0},0,32,XRES,YRES,{0},0,0,0,0,0,0,0,0,0};
    ChangeDisplaySettings( &screenSettings, CDS_FULLSCREEN );
    ShowCursor( 0 );
    HDC hDC = GetDC( CreateWindow( "edit", 0, WS_POPUP|WS_VISIBLE|WS_MAXIMIZE, 0,0,0,0,0,0,0,0 ));

#endif

    SetPixelFormat( hDC, ChoosePixelFormat( hDC, &pfd ), &pfd );
    wglMakeCurrent( hDC, wglCreateContext( hDC ));

#ifdef _DEBUG

    for( int i = 0; i < (sizeof(funcPtrs)/sizeof(void*)); i++ )
        funcPtrs[i] = wglGetProcAddress( funcNames[i] );

    GLchar infolog[1000];
    GLsizei infolen;
    int program = oglCreateProgram();
    int f = oglCreateShader(GL_FRAGMENT_SHADER);	
    oglShaderSource(f, 1, &shader_glsl, 0);
    oglCompileShader(f);
    oglGetShaderInfoLog(f, 1000, &infolen, infolog);
    oglAttachShader(program,f);
    oglLinkProgram(program);
    oglUseProgram(program);

#else

    int program = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))( GL_FRAGMENT_SHADER, 1, &shader_glsl );
    ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))( program );

#endif

    int track = 1;

    for(;;)
    {
        int g[] = {
            0, 0, 0, 0,
            128, 1, 128, 1,
            255, 1, 192, 1,
            0, 100, 0, 7
        };
        unsigned char buffer[16];

        int ballPos = 0;
        int cameraOffset = 0;
        int camFromBall = 0;

        for(;;)
        {
            if( GetAsyncKeyState( VK_ESCAPE )) ExitProcess( 0 );
            if( GetAsyncKeyState( 'R' )) break;
            if( GetAsyncKeyState( 'N' )) { track++; if (track > 10) track = 1; break; }

            g[0] = track;
            g[1] = cameraOffset;
            g[2] = (GetAsyncKeyState(VK_UP)?1:0) + (GetAsyncKeyState(VK_DOWN)?2:0);

            int ballVelX = (256*g[8] + g[9]) - 32768;
            ballPos += ballVelX / 70;
            camFromBall += (ballVelX / 3 - camFromBall) / 99;
            cameraOffset = camFromBall + ballPos;

            g[3] = cameraOffset;

            ((PFNGLUNIFORM4IVPROC)wglGetProcAddress("glUniform4iv"))(
                ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))( program, "g" ),
                4, g
            );

            glRects( -1, -1, 1, 1 );

            glReadPixels( 0, 0, 4, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
            for( int i = 0; i < 16; ++i )
                g[i] = buffer[i];

            SwapBuffers( hDC );
        }
    }

    ExitProcess( 0 );
}
