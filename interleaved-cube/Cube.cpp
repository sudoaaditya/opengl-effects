#include <windows.h>
#include <stdio.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include "../vmath/vmath.h"
#include "Cube.h"
using namespace vmath;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

enum {
    AMK_ATTRIBUTE_POSITION = 0,
    AMK_ATTRIBUTE_COLOR,
    AMK_ATTRIBUTE_NORMAL,
    AMK_ATTRIBUTE_TEXCOORD0
};

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// global variables
bool gbFullScreen = false;
bool gbActiveWindow = false;
DWORD dwStyle = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *fptr = NULL;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

// shader variables
GLuint gVertexShaderObject = 0;
GLuint gFragmentShaderObject = 0;
GLuint gProgramShaderObject = 0;
mat4 perspectiveProjectionMatrix;
GLfloat fAngleCube = 0.0f;
GLuint vao_cube, vbo_position_cube, vbo_normal_cube, vbo_texture_cube;

// uniforms
GLuint modelUniform;
GLuint viewUniform;
GLuint projectionUniform;
GLuint laUniform;
GLuint ldUniform;
GLuint lsUniform;
GLuint lightPosUniform;
GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;
GLuint matShineUniform;
GLuint lKeyPressedUniform;
GLuint samplerUniform;

// Light Parameters
GLfloat lightAmbient[] = { 0.25f, 0.25f, 0.25f, 0.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess = 128.0f;
GLfloat lightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };
bool bLighting = false;
GLuint iMarbleTexture = 0;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
    // local variables & function declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("Interleaved Cube");
    int iRet = 0;
    bool bDone = false;

    void display(void);
    int initialize(void);
    void update(void);
    void ToggleFullScreen(void);

    // code
    if (fopen_s(&fptr, "Log.txt", "w") != 0) {
        MessageBox(NULL, TEXT("Log file can't be created"), TEXT("Error"), MB_OK);
        exit(0);
    } else {
        fprintf(fptr, "Log file created successfully\n");
    }

    // Centered window
    int xPos = GetSystemMetrics(SM_CXSCREEN);
    int yPos = GetSystemMetrics(SM_CYSCREEN);
    int xMid = xPos / 2;
    int yMid = yPos / 2;

    xPos = xMid - (WIN_WIDTH / 2);
    yPos = yMid - (WIN_HEIGHT / 2);

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_MORPHED));
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_MORPHED));
    wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
            szAppName,
            TEXT("OpenGL"),
            WS_OVERLAPPEDWINDOW | 
            WS_CLIPSIBLINGS | 
            WS_CLIPCHILDREN | 
            WS_VISIBLE,
            xPos, yPos,
            WIN_WIDTH, WIN_HEIGHT,
            NULL,
            NULL,
            hInstance,
            NULL
        );

    ghwnd = hwnd;

    iRet = initialize();
    if(iRet == -1) {
        fprintf(fptr, "ChoosePixelFormat Failed!..\n");
        DestroyWindow(hwnd);
    } else if(iRet == -2) {
        fprintf(fptr, "SetPixelFormat Failed!..\n");
        DestroyWindow(hwnd);
    } else if(iRet == -3) {
        fprintf(fptr, "wglCreateContext Failed!..\n");
        DestroyWindow(hwnd);
    } else if(iRet == -4) {
        fprintf(fptr, "wglMakeCurrent Failed!..\n");
        DestroyWindow(hwnd);
    } else if(iRet == -5) {
        fprintf(fptr, "glewInit Failed!..\n");
        DestroyWindow(hwnd);
    } else if(iRet == -6) {
        fprintf(fptr, "Marble Texture Loading Failed!..\n");
        DestroyWindow(hwnd);
    } else {
        fprintf(fptr, "Initialization Successful!..\n");
    }

    ShowWindow(hwnd, iCmdShow);
    SetFocus(hwnd);
    SetForegroundWindow(hwnd);
    ToggleFullScreen();

    while(!bDone) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT) {
                bDone = true;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            if(gbActiveWindow) {
                display();
                update();
            }
        }
    }

    return ((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    // Func & Vars
    void ToggleFullScreen(void);
    void uninitialize(void);
    void resize(int, int);

    static bool bIsMax = false;

    switch(iMsg) {
        case WM_SETFOCUS:
            gbActiveWindow = true;
            break;

        case WM_KILLFOCUS:
            gbActiveWindow = false;
            break;

        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
            break;
            
            case WM_KEYDOWN:
                switch(wParam) {

                    case VK_ESCAPE:
                        DestroyWindow(hwnd);
                        break;
                    
                    case 's':
                    case 'S':
                        if(!bIsMax) {
                            ShowWindow(hwnd, SW_MAXIMIZE);
                            bIsMax = true;
                        }
                        else {
                            ShowWindow(hwnd, SW_SHOWNORMAL);
                            bIsMax = false;
                        }
                    break;

                    case 'f':
                    case 'F':
                        ToggleFullScreen();
                        break;

                    case 'l':
                    case 'L':
                        bLighting = !bLighting;
                        break;
                }
            break;

        case WM_ERASEBKGND:
            return(0);

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            uninitialize();
            PostQuitMessage(0);
            break;

    }

    return(DefWindowProc(hwnd, iMsg, wParam, lParam));

}

void ToggleFullScreen(void){

	//varaaaa
	MONITORINFO mi;

	if(!gbFullScreen){

		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if(dwStyle & WS_OVERLAPPEDWINDOW) {
            mi = {sizeof(MONITORINFO)};
			if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)){

				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOZORDER);
			}
		}
		ShowCursor(FALSE);
		gbFullScreen = TRUE;
	}
	else {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
}


int initialize() {

    //fun & var
    void resize(int,int);
    void uninitialize(void);

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    GLenum result;
    //For Shader Comiple
    GLint iShaderCompileStatus = 0;
    GLint iShaderInfoLogLen = 0;
    GLchar* szInfoLog = NULL;

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize =  sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.cColorBits = 32;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0) {
        return(-1);
    } else {
        fprintf(fptr, "Choose Pixel Format Successful!!..\t{Index = %d}\n", iPixelFormatIndex);
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
        return(-2);
    } else {
        fprintf(fptr, "SetPixelFormat Successful!\n");
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL){
        return(-3);
    } else {
        fprintf(fptr, "wglCreateContext Successsful!!..\n");
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return(-4);
    }
    else {
        fprintf(fptr, "wglMakeCurrent Successful!.\n");
    }

    result = glewInit();
    if(result != GLEW_OK){
        return(-5);
    } else {
        fprintf(fptr, "glewInit Successful!..\n");
    }

    return (0);
}

void resize (int width, int height) {

}

void display () {

}

void update() {

}

void uninitialize() {

}
