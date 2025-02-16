#include <windows.h>
#include <stdio.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include "../vmath/vmath.h"
#include "Cube.h"
#include "clockUtils/Clock.h"
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
GLuint vao_cube, vbo_position_cube, vbo_normal_cube, vbo_texture_cube, vbo_color_cube;

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
GLuint tKeyPressedUniform;
GLuint samplerUniform;
GLuint mvpUniform = 0;

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
bool bTexture = false;
GLuint iMarbleTexture = 0;
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

                    case 't':
                    case 'T':
                        bTexture = !bTexture;
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
    } else {
        fprintf(fptr, "wglMakeCurrent Successful!.\n");
    }

    result = glewInit();
    if(result != GLEW_OK){
        return(-5);
    } else {
        fprintf(fptr, "glewInit Successful!..\n");
    }

    //Shader Code : Define Vertex Shader Object
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    // Write Vertex Shader Code!.
    const GLchar *vertexShaderSourceCode = 
        "#version 460 core" \
        "\n" \
        "in vec4 vPosition;" \
        "in vec4 vColor;" \
        "in vec3 vNormal;" \
        "in vec2 vTexCoord;" \
        "uniform mat4 u_modelMatrix;" \
        "uniform mat4 u_viewMatrix;" \
        "uniform mat4 u_projMatrix;" \
        "uniform int u_lPressed;" \
        "uniform vec4 u_lightPosition;" \
        "out vec4 out_color;" \
        "out vec2 out_texCoord;" \
        "out vec3 out_normal;" \
        "out vec3 out_viewerVector;" \
        "out vec3 out_lightDirection;" \
        "void main(void) {" \
        "   if(u_lPressed == 1) {" \
        "       vec4 eyeCoords = u_viewMatrix * u_modelMatrix * vPosition;" \
        "       out_normal = mat3(u_viewMatrix * u_modelMatrix) * vNormal;" \
        "       out_lightDirection = vec3(u_lightPosition - eyeCoords);" \
        "       out_viewerVector = vec3(-eyeCoords.xyz);" \
        "   }" \
        "   gl_Position = u_projMatrix * u_viewMatrix * u_modelMatrix * vPosition;" \
        "   out_color = vColor;" \
        "   out_texCoord = vTexCoord;" \
        "}";

    // Specify above code to VertexShader Object.
    glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    //Compile The Vertex Shader.
    glCompileShader(gVertexShaderObject);

    //Error Checking.
    glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

    if(iShaderCompileStatus == GL_FALSE) {
        glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);

        if(iShaderInfoLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iShaderInfoLogLen);

            if(szInfoLog != NULL) {
                GLsizei written;

                glGetShaderInfoLog(gVertexShaderObject,
                    iShaderInfoLogLen,
                    &written,
                    szInfoLog);

                fprintf(fptr, "Vertex Shader Log::\n %s\n", szInfoLog);

                free(szInfoLog);
                uninitialize();
                DestroyWindow(ghwnd);
                exit(0);
            }
        }
    } else {
        fprintf(fptr, "Vertex Shader Compiled Successfully!!..\n");
    }

    //Fragment Shader : create Shader Objet!
    iShaderCompileStatus = 0;
    iShaderInfoLogLen = 0;
    szInfoLog = NULL;
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *fragmentShaderSourceCode = 
        "#version 460 core" \
        "\n" \
        "uniform vec3 u_la;" \
        "uniform vec3 u_ld;" \
        "uniform vec3 u_ls;" \
        "uniform vec3 u_ka;" \
        "uniform vec3 u_kd;" \
        "uniform vec3 u_ks;" \
        "uniform sampler2D u_sampler;" \
        "uniform float u_matShine;" \
        "uniform int u_tPressed;" \
        "uniform int u_lPressed;" \
        "in vec4 out_color;" \
        "in vec2 out_texCoord;" \
        "in vec3 out_normal;" \
        "in vec3 out_lightDirection;" \
        "in vec3 out_viewerVector;" \
        "out vec4 FragColor;" \
        "void main(void) {" \
        "   vec3 phong_ads_light;" \
        "   if(u_lPressed == 1) {" 
        "       vec3 n_normal = normalize(out_normal);" \
        "       vec3 n_lightDirection = normalize(out_lightDirection);" \
        "       vec3 n_viewerVector = normalize(out_viewerVector);" \
        "       float tn_dot_ld = max(dot(n_lightDirection, n_normal), 0.0);" \
        "       vec3 reflectionVector = reflect(-n_lightDirection, n_normal);"  \
        "       vec3 ambient = u_la * u_ka;" \
        "       vec3 diffuse = u_ld * u_kd * tn_dot_ld;" \
        "       vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector, n_viewerVector), 0.0), u_matShine);" \
        "       phong_ads_light = ambient + diffuse + specular;" \
        "   } else {" \
        "       phong_ads_light = vec3(1.0, 1.0, 1.0);" \
        "   }" \
        "   if(u_tPressed == 1) {" \
        "       vec3 tex = texture(u_sampler, out_texCoord).rgb;" \
        "       phong_ads_light *= out_color.rgb * tex;" \
        "   } else {" \
        "       phong_ads_light *= out_color.rgb;" \
        "   }"
        "   FragColor = vec4(phong_ads_light, 1.0);" \
        "}";

    glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

    glCompileShader(gFragmentShaderObject);

    glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

    if(iShaderCompileStatus == GL_FALSE) {
        glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);

        if(iShaderInfoLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iShaderInfoLogLen);
            if(szInfoLog != NULL) {
                GLint written;

                glGetShaderInfoLog(gVertexShaderObject,
                                iShaderInfoLogLen,
                                &written,
                                szInfoLog);

                fprintf(fptr,"Fragment Shader Log :\n %s\n",szInfoLog);
                free(szInfoLog);
                uninitialize();
                DestroyWindow(ghwnd);
                exit(0);
            }
        }
    } else {
        fprintf(fptr, "Fragement ShaderCompiled Successfully!!..\n");
    }

    //Noe Create Program
    GLint iProgLinkStatus = 0;
    GLint iProgLogLen = 0;
    GLchar* szProgLog = NULL;
    //Create Shader Program Shader Object!
    gProgramShaderObject = glCreateProgram();

    //Attach VS to Shader Prog
    glAttachShader(gProgramShaderObject, gVertexShaderObject);

    //Attach FS to Shader Prog
    glAttachShader(gProgramShaderObject, gFragmentShaderObject);

    //NOW BEFORE LINK : Prelinking Binding with Vertex Attribute
    glBindAttribLocation(gProgramShaderObject, AMK_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gProgramShaderObject, AMK_ATTRIBUTE_COLOR, "vColor");
    glBindAttribLocation(gProgramShaderObject, AMK_ATTRIBUTE_NORMAL, "vNormal");
    glBindAttribLocation(gProgramShaderObject, AMK_ATTRIBUTE_TEXCOORD0, "vTexCoord");

    //Now Link The Program
    glLinkProgram(gProgramShaderObject);

    glGetProgramiv(gProgramShaderObject, GL_LINK_STATUS, &iProgLinkStatus);

    if(iProgLinkStatus == GL_FALSE) {
        glGetProgramiv(gProgramShaderObject, GL_INFO_LOG_LENGTH, &iProgLogLen);

        if(iProgLogLen > 0) {
            szProgLog = (GLchar*)malloc(iProgLogLen);

            if(szProgLog != NULL) {
                GLint written;

                glGetProgramInfoLog(gProgramShaderObject, iProgLogLen, &written, szProgLog);

                fprintf(fptr,"Program Link Log :\n %s\n",szProgLog);

                uninitialize();
                DestroyWindow(ghwnd);
                exit(0);
            }
        }
    } else {
        fprintf(fptr, "Program Linkage Successful!!\n");
    }

    // uniforms
    modelUniform = glGetUniformLocation(gProgramShaderObject, "u_modelMatrix");
    viewUniform = glGetUniformLocation(gProgramShaderObject, "u_viewMatrix");
    projectionUniform = glGetUniformLocation(gProgramShaderObject, "u_projMatrix");
    samplerUniform = glGetUniformLocation(gProgramShaderObject, "u_sampler");
    tKeyPressedUniform = glGetUniformLocation(gProgramShaderObject, "u_tPressed");
    laUniform = glGetUniformLocation(gProgramShaderObject, "u_la");
    ldUniform = glGetUniformLocation(gProgramShaderObject, "u_ld");
    lsUniform = glGetUniformLocation(gProgramShaderObject, "u_ls");
    kaUniform = glGetUniformLocation(gProgramShaderObject, "u_ka");
    kdUniform = glGetUniformLocation(gProgramShaderObject, "u_kd");
    ksUniform = glGetUniformLocation(gProgramShaderObject, "u_ks");
    matShineUniform = glGetUniformLocation(gProgramShaderObject, "u_matShine");
    lKeyPressedUniform = glGetUniformLocation(gProgramShaderObject, "u_lPressed");
    lightPosUniform = glGetUniformLocation(gProgramShaderObject, "u_lightPosition");

    // Arrays
    const GLfloat cubeVertices[] = { 
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f
    };

    const GLfloat cubeNormals[] = {
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  
        0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  
        0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  
        -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f  
    };

    const GLfloat cubeColor[] = { 
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f
    };

    const GLfloat cubeTexCoord[] = { 
        1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.01, 1.0f, 0.0f, 0.0f, 
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f  
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

    //Normals
    glGenBuffers(1, &vbo_normal_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMK_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMK_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

    //TexCoords
    glGenBuffers(1, &vbo_texture_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexCoord), cubeTexCoord, GL_STATIC_DRAW);
    glVertexAttribPointer(AMK_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMK_ATTRIBUTE_TEXCOORD0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if(loadTexture(&iMarbleTexture, MAKEINTRESOURCE(TEXTURE_MARBLE)) == FALSE) {
        return(-6);
    } else {
        fprintf(fptr, "Marble Texture Loaded Sussessfully!\n");
    }

    perspectiveProjectionMatrix = mat4::identity();

    resize(WIN_WIDTH, WIN_HEIGHT);

    myClock.start();

    return (0);
}

BOOL loadTexture(GLuint *texture, TCHAR imgResourceID[]) {
    HBITMAP hBitmap = NULL;
    BITMAP bmp;
    BOOL bStatus = FALSE;

    hBitmap = (HBITMAP)LoadImage(
        GetModuleHandle(NULL),
        imgResourceID,
        IMAGE_BITMAP,
        0, 0,
        LR_CREATEDIBSECTION
    );

    if(hBitmap) {
        bStatus = TRUE;

        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(
            GL_TEXTURE_2D, 
            0, GL_RGBA,
            bmp.bmWidth, bmp.bmHeight, 
            0, GL_BGR_EXT, GL_UNSIGNED_BYTE,
            bmp.bmBits
        );

        glGenerateMipmap(GL_TEXTURE_2D);
        DeleteObject(hBitmap);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return(bStatus);
}

void resize (int width, int height) {
    if(height == 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgramShaderObject);

    modelMat = mat4::identity();
    viewMat = mat4::identity();
    translateMat = mat4::identity();
    rotateMat = mat4::identity();
    scaleMat = mat4::identity();

    translateMat = translate(0.0f, 0.0f, -6.0f);
    scaleMat = scale(0.75f, 0.75f, 0.75f);

    rotateMat = rotate(fAngleCube, 1.0f, 0.0f, 0.0f);
    rotateMat *= rotate(fAngleCube, 0.0f, 1.0f, 0.0f);
    rotateMat *= rotate(fAngleCube, 0.0f, 0.0f, 1.0f);

    modelMat = modelMat * translateMat;
    modelMat = modelMat * scaleMat;
    modelMat = modelMat * rotateMat;

    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMat);
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    if(bLighting) {
        glUniform3fv(laUniform, 1, lightAmbient);
        glUniform3fv(ldUniform, 1, lightDiffuse);
        glUniform3fv(lsUniform, 1, lightSpecular);
        glUniform3fv(kaUniform, 1, materialAmbient);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform3fv(ksUniform, 1, materialSpecular);
        glUniform1f(matShineUniform, materialShininess);
        glUniform4fv(lightPosUniform, 1, lightPosition);
        glUniform1i(lKeyPressedUniform, 1);
    } else {
        glUniform1i(lKeyPressedUniform, 0);
    }

    glBindVertexArray(vao_cube);
    
    if(bTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, iMarbleTexture);
        glUniform1i(samplerUniform, 0);
        glUniform1i(tKeyPressedUniform, 1);
    } else {
        glActiveTexture(0);
        glUniform1i(tKeyPressedUniform, 0);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

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

    if(vbo_texture_cube) {
        glDeleteBuffers(1, &vbo_texture_cube);
        vbo_texture_cube = 0;
    }

    if(vbo_color_cube) {
        glDeleteBuffers(1, &vbo_color_cube);
        vbo_color_cube = 0;
    }

    if(vbo_normal_cube) {
        glDeleteBuffers(1, &vbo_normal_cube);
        vbo_normal_cube = 0;
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
        GLsizei iShaderCnt = 0;
        GLsizei iShaderNo = 0;

        glUseProgram(gProgramShaderObject);

        glGetProgramiv(
            gProgramShaderObject,
            GL_ATTACHED_SHADERS,
            &iShaderCnt
        );

        GLuint *pShaders = (GLuint*)malloc(iShaderCnt * sizeof(GLuint));

        if(pShaders) {
            glGetAttachedShaders(
                gProgramShaderObject,
                iShaderCnt, 
                &iShaderCnt, 
                pShaders
            );

            for(iShaderNo = 0; iShaderNo < iShaderCnt; iShaderNo++) {
                glDetachShader(gProgramShaderObject, pShaders[iShaderNo]);
                fprintf(fptr, "Detached Shader: %ld\n", pShaders[iShaderNo]);
                pShaders[iShaderNo] = 0;
            }
            free(pShaders);
        }

        glDeleteProgram(gProgramShaderObject);
        gProgramShaderObject = 0;
        glUseProgram(0);
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
        fclose(fptr);
        fptr = NULL;
    }
}
