#include <windows.h>
#include <stdio.h>
#include <gl/glew.h>
#include <gl/GL.h>

#include "../vmath/vmath.h"
#include "AACube.h"
#include "clockUtils/Clock.h"
#include "shaderUtils/LoadShaders.h"

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
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// global variables
bool gbFullScreen = false;
bool gbActiveWindow = false;
bool gbToggleAliasing = false;
DWORD dwStyle = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
FILE *fptr = NULL;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

// shader variables
GLuint gProgramShaderObject = 0;
GLuint gProgramPostProcShaderObject = 0;

mat4 perspectiveProjectionMatrix;
GLfloat fAngleCube = 0.0f;
GLuint vao_cube, vbo_position_cube, vbo_color_cube;
GLuint vao_quad, vbo_position_quad, vbo_texture_quad;

// uniforms
GLuint modelUniform;
GLuint viewUniform;
GLuint projectionUniform;
GLuint samplerUniform;
GLuint mvpMatrixUniform;
GLuint toggleAliasing;

GLuint framebuffer = 0;
GLuint renderbuffer = 0;
GLuint intermediateFBO = 0;
GLuint screenTexture = 0;
GLuint textureColorBufferMultiSampled = 0;

unsigned int samples = 4;

Clock myClock;

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
        fflush(fptr);
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
    wndclass.hbrBackground = CreateSolidBrush(RGB(1, 0, 0));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
            szAppName,
            TEXT("AMK_OpenGL"),
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
        fflush(fptr);
        DestroyWindow(hwnd);
    } else if(iRet == -2) {
        fprintf(fptr, "SetPixelFormat Failed!..\n");
        fflush(fptr);
        DestroyWindow(hwnd);
    } else if(iRet == -3) {
        fprintf(fptr, "wglCreateContext Failed!..\n");
        fflush(fptr);
        DestroyWindow(hwnd);
    } else if(iRet == -4) {
        fprintf(fptr, "wglMakeCurrent Failed!..\n");
        fflush(fptr);
        DestroyWindow(hwnd);
    } else if(iRet == -5) {
        fprintf(fptr, "glewInit Failed!..\n");
        fflush(fptr);
        DestroyWindow(hwnd);
    } else {
        fprintf(fptr, "Initialization Successful!..\n");
        fflush(fptr);
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

                    case 'a':
                    case 'A':
                        gbToggleAliasing = !gbToggleAliasing;
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
	//var
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
		gbFullScreen = true;
	}
	else {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = false;
	}
}


int initialize() {

    //fun & var
    void resize(int,int);
    void uninitialize(void);
    BOOL loadTexture(GLuint*, TCHAR[]);
    // void glPrintError(char *);

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
        fflush(fptr);
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
        return(-2);
    } else {
        fprintf(fptr, "SetPixelFormat Successful!\n");
        fflush(fptr);
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL){
        return(-3);
    } else {
        fprintf(fptr, "wglCreateContext Successsful!!..\n");
        fflush(fptr);
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return(-4);
    } else {
        fprintf(fptr, "wglMakeCurrent Successful!.\n");
        fflush(fptr);
    }

    result = glewInit();
    if(result != GLEW_OK){
        return(-5);
    } else {
        fprintf(fptr, "glewInit Successful!..\n");
        fflush(fptr);
    }

    //Shader Code : Define Vertex Shader Object
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "./shaders/anti-vertex.vert" },
        { GL_FRAGMENT_SHADER, "./shaders/anti-fragment.frag" },
        { GL_NONE , ""}
    };

    gProgramShaderObject = LoadShaders(shaders, fptr);

    //NOW BEFORE LINK : Prelinking Binding with Vertex Attribute
    glBindAttribLocation(gProgramShaderObject, AMK_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gProgramShaderObject, AMK_ATTRIBUTE_COLOR, "vColor");

    // uniforms
    modelUniform = glGetUniformLocation(gProgramShaderObject, "u_modelMatrix");
    viewUniform = glGetUniformLocation(gProgramShaderObject, "u_viewMatrix");
    projectionUniform = glGetUniformLocation(gProgramShaderObject, "u_projMatrix");

    // Arrays
    const GLfloat cubeVertices[] = { 
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f
    };

    const GLfloat cubeColor[] = { 
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);

    //Vertices
    glGenBuffers(1, &vbo_position_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMK_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMK_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Color
    glGenBuffers(1, &vbo_color_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColor), cubeColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMK_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMK_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Post Processing Shader
    ShaderInfo postShaders[] = {
        { GL_VERTEX_SHADER, "./shaders/post-vertex.vert" },
        { GL_FRAGMENT_SHADER, "./shaders/post-fragment.frag" },
        { GL_NONE , ""}
    };

    gProgramPostProcShaderObject = LoadShaders(postShaders, fptr);

    //NOW BEFORE LINK : Prelinking Binding with Vertex Attribute
    glBindAttribLocation(gProgramPostProcShaderObject, AMK_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(gProgramPostProcShaderObject, AMK_ATTRIBUTE_TEXCOORD0, "aTexCoords");

    // glPrintError("433: Before BlitFramebuffer");

    // uniforms
    samplerUniform = glGetUniformLocation(gProgramPostProcShaderObject, "screenTex");
    mvpMatrixUniform = glGetUniformLocation(gProgramPostProcShaderObject, "mvpMatrix");
    toggleAliasing = glGetUniformLocation(gProgramPostProcShaderObject, "toggleAliasing");

    float rectPosCoords[]  = { 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f};
    float rectTexCoords[]  = { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };

    glGenVertexArrays(1, &vao_quad);
	glBindVertexArray(vao_quad);

    glGenBuffers(1, &vbo_position_quad);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectPosCoords), &rectPosCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(AMK_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMK_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_texture_quad);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectTexCoords), rectTexCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(AMK_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMK_ATTRIBUTE_TEXCOORD0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0);

    // FBO MAX SIZES
    int maxWidth = GetSystemMetrics(SM_CXSCREEN);
    int maxHeight = GetSystemMetrics(SM_CYSCREEN);

    fprintf(fptr, "FBO Width: %d, Height: %d\n", maxWidth, maxHeight);  
    fflush(fptr);

    // Configure MSAA framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // Create a multisampled color attachment texture
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, maxWidth, maxHeight, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // Create a multisampled renderbuffer for depth and stencil attachments
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, maxWidth, maxHeight);
    // glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(fptr, "ERROR: Framebuffer is not complete!\n");
        fflush(fptr);
    } else {
        fprintf(fptr, "\nFramebuffer is complete!\n");
        fflush(fptr);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);

    // glPrintError("517: Before BlitFramebuffer");

    perspectiveProjectionMatrix = mat4::identity();

    // resize(WIN_WIDTH, WIN_HEIGHT);

    myClock.start();

    return (0);
}

void resize (int width, int height) {

    // void glPrintError(char *);

    if(height == 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    // glPrintError("553");
    // fprintf(fptr, "WIN WIDTH & HEIGHT %d, %d\n", width, height);
    // fflush(fptr);

    perspectiveProjectionMatrix = perspective(
        45.0f,
        ((GLfloat)width / (GLfloat)height),
        0.1f,
        100.0f
    );
}

void display () {

    // local vars
    mat4 modelMat;
    mat4 viewMat;
    mat4 translateMat;
    mat4 rotateMat;
    mat4 scaleMat;

    // void glPrintError(char *);

    // glPrintError("551: glClearColor");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glPrintError("554: Before BlitFramebuffer");

    // Step 1: draw scene in multisampled buffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // glPrintError("560: Before BlitFramebuffer");

    glUseProgram(gProgramShaderObject);

    modelMat = mat4::identity();
    viewMat = mat4::identity();
    translateMat = mat4::identity();
    rotateMat = mat4::identity();
    scaleMat = mat4::identity();

    translateMat = translate(0.0f, 0.0f, -7.0f);
    scaleMat = scale(1.2f, 1.2f, 1.2f);

    rotateMat = rotate(fAngleCube, 1.0f, 0.0f, 0.0f);
    rotateMat *= rotate(fAngleCube, 0.0f, 1.0f, 0.0f);
    rotateMat *= rotate(fAngleCube, 0.0f, 0.0f, 1.0f);

    modelMat = modelMat * translateMat;
    modelMat = modelMat * scaleMat;
    modelMat = modelMat * rotateMat;

    // glPrintError("579: Before BlitFramebuffer");
    
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMat);
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    // glPrintError("581: Before BlitFramebuffer");

    glBindVertexArray(vao_cube);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Step 3: Now render quad with scene's visual as it's texture
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(gProgramPostProcShaderObject);    

    modelMat = mat4::identity();
    viewMat = mat4::identity();
    translateMat = mat4::identity();

    translateMat = translate(0.0f, 0.0f, -3.0f);

    modelMat = perspectiveProjectionMatrix * viewMat * translateMat;

    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelMat);

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glUniform1i(samplerUniform, 0);

    glUniform1i(toggleAliasing, gbToggleAliasing ? 1 : 0);

    glBindVertexArray(vao_quad);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);

    SwapBuffers(ghdc);
}

void update() {
    float elapsedTime = (float) myClock.getElapsedTime();
    fAngleCube = fmod((elapsedTime * 30.0f), 360.0f);
    if(fAngleCube >= 360.0f) {
        fAngleCube = 0.0f;
    }
}

void glPrintError(char *strMsg) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        fprintf(fptr, "glError: %s : [%d] %s \n", strMsg, err, gluErrorString( err ));
        fflush(fptr);
    }
}

void uninitialize() {
    if(gbFullScreen) {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(ghwnd, &wpPrev);

        SetWindowPos(ghwnd, 
            HWND_TOP,
            0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);    

        ShowCursor(TRUE);
        gbFullScreen = false;
    }

    if(vbo_color_cube) {
        glDeleteBuffers(1, &vbo_color_cube);
        vbo_color_cube = 0;
    }

    if(vbo_position_cube) {
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }

    if(vao_cube) {
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
    }

    if(gProgramShaderObject) {
        GLuint iRet = DetachShaders(gProgramShaderObject, fptr);
        if(iRet != 0) {
            fprintf(fptr, "gProgramShaderObject: Failed to Detach Shaders!\n");
            fflush(fptr);
        } else {
            fprintf(fptr, "gProgramShaderObject: Shaders Detached Successfully!\n\n");
            fflush(fptr);
        }
    }

    if(vbo_position_quad) {
        glDeleteBuffers(1, &vbo_position_quad);
        vbo_position_quad = 0;
    }

    if(vao_quad) {
        glDeleteVertexArrays(1, &vao_quad);
        vao_quad = 0;
    }

    if(gProgramPostProcShaderObject) {
        GLuint iRet = DetachShaders(gProgramPostProcShaderObject, fptr);
        if(iRet != 0) {
            fprintf(fptr, "gProgramPostProcShaderObject: Failed to Detach Shaders!\n");
            fflush(fptr);
        } else {
            fprintf(fptr, "gProgramPostProcShaderObject: Shaders Detached Successfully!\n\n");
            fflush(fptr);
        }
    }

    if(ghrc == wglGetCurrentContext()) {
        wglMakeCurrent(NULL, NULL);
    }

    if(ghrc) {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    if(ghdc) {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    if(fptr) {
        fprintf(fptr, "\nFile Closed Successfully!!..\n");
        fflush(fptr);
        fclose(fptr);
        fptr = NULL;
    }
}
