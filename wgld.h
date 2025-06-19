#ifndef WGLD_H
#define WGLD_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/glu.h>
#include <gl/gl.h>

typedef struct {

    struct {
        WORD Width, Height;
        char* Title;
    } Option;

    struct {
        MSG Msg;
        int CmdShow;
        HWND Handle;
        HDC DeviceContext;
        LPCWSTR ClassName;
        HINSTANCE AppInstance;
        HGLRC RenderingContext;
    } Windows;

    struct {
        BOOL depth;
    } Opengl;

} WinGL_Device;
typedef void (*WinGL_MainloopCallback)(void);
typedef void (*WinGL_KeyCallback)(WPARAM key, LPARAM flags);
typedef void (*WinGL_MouseCallback)(int x, int y, WPARAM button, LPARAM flags);

WinGL_Device* WinGL_Init(char* title, int width, int height, BOOL depth, HINSTANCE hInstance, int nCmdShow);
void WinGL_Mainloop(WinGL_Device* wgld, WinGL_MainloopCallback mainloop);
void WinGL_Shutdown(WinGL_Device* wgld);
double WinGL_GetTime();
float WinGL_GetDelta();

#endif