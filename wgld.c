#include "wgld.h"
#include <stdlib.h>

struct WinGL_Timing {
    LARGE_INTEGER freq;
    LARGE_INTEGER last;
    DWORD lastTick;
    float deltaTime;    // Son frame süresi saniye cinsinden
    double totalTime;    // Toplam geçen zaman saniye cinsinden
} time;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            glViewport(0, 0, width, height);
        }
        return 0;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

static PIXELFORMATDESCRIPTOR S_CreatePFD() {
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    return pfd;
}

static void S_BindRendererContext(HWND* hwnd, HDC* hdc, HGLRC* hglrc, PIXELFORMATDESCRIPTOR* pfd) {
    *hdc = GetDC(*hwnd);
    SetPixelFormat(*hdc, ChoosePixelFormat(*hdc, pfd), pfd);
    *hglrc = wglCreateContext(*hdc);
    wglMakeCurrent(*hdc, *hglrc);
}

static void S_AdjustWindowBar(HWND hwnd, int clientWidth, int clientHeight) {
    RECT rect = {0, 0, clientWidth, clientHeight};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    SetWindowPos(hwnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
}

static void S_StartOpenGL(WinGL_Device* wgld) {
    // SETUP SHADING (FLAT-SMOOTH)
    glShadeModel(GL_FLAT);
    // SETUP VIEWPORT
    glViewport(0, 0, wgld->Option.Width, wgld->Option.Height);
    // SETUP PROJECTION
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (wgld->Opengl.depth) {
        gluPerspective(45.0, (GLfloat)wgld->Option.Width / (GLfloat)wgld->Option.Height, 1.0, 1000.0);
        glEnable(GL_DEPTH_TEST);
    } else {
        glOrtho(0, wgld->Option.Width, wgld->Option.Height, 0, -1, 1);
        glDisable(GL_DEPTH_TEST);
    }
    // SETUP MODEL
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

WinGL_Device* WinGL_Init(char* title, int width, int height, BOOL depth, HINSTANCE hInstance, int nCmdShow)
{
    WinGL_Device* wgld = malloc(sizeof(WinGL_Device));
    if (!wgld) return NULL;

    wgld->Opengl.depth = depth;

    wgld->Option.Width = width;
    wgld->Option.Height = height;
    wgld->Option.Title = title;

    wgld->Windows.AppInstance = hInstance;
    wgld->Windows.CmdShow = nCmdShow;
    wgld->Windows.ClassName = L"OpenGLWinClass";

    WNDCLASSW wc = {0};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = wgld->Windows.AppInstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wgld->Windows.ClassName;

    if (!RegisterClassW(&wc)) {
        free(wgld);
        return NULL;
    }

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    wgld->Windows.Handle = CreateWindowW(
        wgld->Windows.ClassName,
        (wchar_t*)wgld->Option.Title,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, wgld->Option.Width, wgld->Option.Height,
        NULL, NULL, wgld->Windows.AppInstance, NULL
    );
    if (wgld->Windows.Handle == NULL) {
        free(wgld);
        return NULL;
    }

    S_AdjustWindowBar(wgld->Windows.Handle, width, height);

    PIXELFORMATDESCRIPTOR pfd = S_CreatePFD();
    S_BindRendererContext(&wgld->Windows.Handle, &wgld->Windows.DeviceContext, &wgld->Windows.RenderingContext, &pfd);
    ShowWindow(wgld->Windows.Handle, wgld->Windows.CmdShow);
    S_StartOpenGL(wgld);

    return wgld;
}

void WinGL_Mainloop(WinGL_Device* wgld, WinGL_MainloopCallback mainloop)
{
    QueryPerformanceFrequency(&time.freq);
    QueryPerformanceCounter(&time.last);

    while (1) {
        if (PeekMessage(&wgld->Windows.Msg, NULL, 0, 0, PM_REMOVE)) {
            if (wgld->Windows.Msg.message == WM_QUIT) break;
            TranslateMessage(&wgld->Windows.Msg);
            DispatchMessage(&wgld->Windows.Msg);
        } else {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);

            time.deltaTime = (float)(now.QuadPart - time.last.QuadPart) / time.freq.QuadPart;
            time.totalTime += time.deltaTime;
            time.last = now;

            mainloop();
            SwapBuffers(wgld->Windows.DeviceContext);

            // 60 FPS için hedef frame süresi (saniye)
            const double targetFrameTime = 1.0 / 60.0;
            double remainingTime = targetFrameTime - time.deltaTime;

            if (remainingTime > 0) {
                Sleep((DWORD)(remainingTime * 1000));
            }
        }
    }
}

void WinGL_Shutdown(WinGL_Device *wgld)
{
    if (!wgld) return;
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(wgld->Windows.RenderingContext);
    ReleaseDC(wgld->Windows.Handle, wgld->Windows.DeviceContext);
    DestroyWindow(wgld->Windows.Handle);
    UnregisterClassW(wgld->Windows.ClassName, wgld->Windows.AppInstance);
    free(wgld);
}

double WinGL_GetTime()
{
    return time.totalTime;
}

float WinGL_GetDelta()
{
    return time.deltaTime;
}
