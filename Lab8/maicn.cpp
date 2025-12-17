// Test.cpp
//
#define STRICT
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <Windows.h>
#include <string>
#include "resource.h"

#define SCLASS "a;eohgqeruiopugoqeig"
#define WINDOW_WIDTH 0.5
#define WINDOW_HEIGHT 0.5

#define M 300
#define N 400

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI  WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE, // hPrevInstance,
    _In_ LPSTR, // lpCmdLine,
    _In_ int nCmdShow
)
{
    LPCTSTR szClass = SCLASS;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szClass;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (::RegisterClass(&wc) == 0)
    {
        return -1;
    }

    double screenWidth = GetSystemMetrics(SM_CXSCREEN);
    double screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create Main window
    HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
    HWND overlappedWindow = ::CreateWindow(szClass, "MainWindow", WS_OVERLAPPEDWINDOW,
        (int)((screenWidth - (screenWidth * WINDOW_WIDTH)) / 2),
        (int)((screenHeight - (screenHeight * WINDOW_HEIGHT)) / 2),
        (int)(screenWidth * WINDOW_WIDTH), (int)(screenHeight * WINDOW_HEIGHT),
        NULL, hMenu, hInstance, NULL);
    if (overlappedWindow == NULL) {
        return -1;
    }

    ::ShowWindow(overlappedWindow, nCmdShow);

    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0)) {
        ::DispatchMessage(&msg);
    }

    return 0;
}
//=========================================================

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BYTE buffer[M][N] = {};
    static RECT drawRect = { 0, 0, N, M };
    static bool useLineMode = false;

    static bool  hasPrevMove = false;
    static POINT prevMovePt = { 0, 0 };

    static bool  hasPrevClick = false;
    static POINT prevClickPt = { 0, 0 };

    switch (message) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_FILE_EXIT: {
            DestroyWindow(hWnd);
            return 0;
        }
        case ID_MODE_FOLLOWING: {
            useLineMode = false;
            return 0;
        }
        case ID_MODE_LINE: {
            useLineMode = true;
            return 0;
        }
        }
        return 0;
    }
    case WM_LBUTTONDOWN: {
        if (useLineMode) {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            if (PtInRect(&drawRect, pt)) {
                HDC hDc = GetDC(hWnd);

                if (!hasPrevClick) {
                    prevClickPt = pt;
                    hasPrevClick = true;
                }
                else {
                    int dx = pt.x - prevClickPt.x;
                    int dy = pt.y - prevClickPt.y;
                    int steps = max(abs(dx), abs(dy));
                    if (steps == 0) {
                        SetPixelV(hDc, pt.x, pt.y, RGB(0, 0, 0));
                        buffer[pt.y - drawRect.top][pt.x - drawRect.left] = 255;
                    }
                    else {
                        double sx = dx / (double)steps;
                        double sy = dy / (double)steps;
                        double x = prevClickPt.x, y = prevClickPt.y;
                        for (int i = 0; i <= steps; ++i) {
                            POINT p = { (LONG)lround(x), (LONG)lround(y) };
                            if (PtInRect(&drawRect, p)) {
                                SetPixelV(hDc, p.x, p.y, RGB(0, 0, 0));
                                buffer[p.y - drawRect.top][p.x - drawRect.left] = 255;
                            }
                            x += sx; y += sy;
                        }
                    }
                    prevClickPt = pt;
                }

                ReleaseDC(hWnd, hDc);
            }
            return 0;
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (!useLineMode && (wParam & MK_LBUTTON)) {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            if (PtInRect(&drawRect, pt)) {
                HDC hDc = GetDC(hWnd);
                if (!hasPrevMove) {
                    SetPixelV(hDc, pt.x, pt.y, RGB(0, 0, 0));
                    buffer[pt.y - drawRect.top][pt.x - drawRect.left] = 255;
                }
                else {
                    int dx = pt.x - prevMovePt.x;
                    int dy = pt.y - prevMovePt.y;
                    int steps = max(abs(dx), abs(dy));
                    if (steps == 0) {
                        SetPixelV(hDc, pt.x, pt.y, RGB(0, 0, 0));
                        buffer[pt.y - drawRect.top][pt.x - drawRect.left] = 255;
                    }
                    else {
                        double sx = dx / (double)steps;
                        double sy = dy / (double)steps;
                        double x = prevMovePt.x, y = prevMovePt.y;
                        for (int i = 0; i <= steps; ++i) {
                            POINT p = { (LONG)lround(x), (LONG)lround(y) };
                            if (PtInRect(&drawRect, p)) {
                                SetPixelV(hDc, p.x, p.y, RGB(0, 0, 0));
                                buffer[p.y - drawRect.top][p.x - drawRect.left] = 255;
                            }
                            x += sx; y += sy;
                        }
                    }
                }
                ReleaseDC(hWnd, hDc);
                prevMovePt = pt;
                hasPrevMove = true;
            }
        }
        else {
            hasPrevMove = false;
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hDc = BeginPaint(hWnd, &ps);
        Rectangle(hDc, drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);
        for (int y = 0; y < M; y++) {
            for (int x = 0; x < N; x++) {
                if (buffer[y][x]) {
                    SetPixelV(hDc, drawRect.left + x, drawRect.top + y, RGB(0, 0, 0));
                }
            }
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZING: {
        RECT* pRect = (RECT*)lParam;
        int minW = (int)ceil(1.2 * N);
        int minH = (int)ceil(1.2 * M);
        RECT req = { 0,0,minW,minH };
        AdjustWindowRectEx(&req, GetWindowLong(hWnd, GWL_STYLE),
            (GetMenu(hWnd) != NULL), GetWindowLong(hWnd, GWL_EXSTYLE));
        int minWinW = req.right - req.left;
        int minWinH = req.bottom - req.top;
        if ((pRect->right - pRect->left) < minWinW)
            pRect->right = pRect->left + minWinW;
        if ((pRect->bottom - pRect->top) < minWinH)
            pRect->bottom = pRect->top + minWinH;
        return TRUE;
    }
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}
//=========================================================
