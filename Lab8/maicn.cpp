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

#define N 400
#define M 300

struct AppState {
    BYTE pixels[M][N];
    RECT drawRect;
    POINT prevPoint;
    bool hasPrevPt;
	bool lineMode;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void Draw(HWND hWnd, AppState* state, POINT mPt);

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

void Draw(HWND hWnd, AppState* state, POINT mPt) {
    if (PtInRect(&state->drawRect, mPt)) {
        HDC hdc = GetDC(hWnd);
        if (state->hasPrevPt) {
            POINT d;
            d.x = mPt.x - state->prevPoint.x;
            d.y = mPt.y - state->prevPoint.y;
            int steps = max(abs(d.x), abs(d.y));
            if (steps == 0) {
                SetPixelV(hdc, mPt.x, mPt.y, RGB(0, 0, 0));
                state->pixels[mPt.y][mPt.x] = 1;
            }
            else {
                POINT pt;
                for (int i = 0; i <= steps; ++i) {
                    pt.x = state->prevPoint.x + i * d.x / steps;
                    pt.y = state->prevPoint.y + i * d.y / steps;
                    SetPixelV(hdc, pt.x, pt.y, RGB(0, 0, 0));
                    state->pixels[pt.y][pt.x] = 1;
                }
            }
        }
        else {
            SetPixelV(hdc, mPt.x, mPt.y, RGB(0, 0, 0));
            state->pixels[mPt.y][mPt.x] = 1;
        }
        state->prevPoint = mPt;
        state->hasPrevPt = true;
        ReleaseDC(hWnd, hdc);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE: {
        AppState* state = new AppState{};
        state->drawRect = { 0, 0, N, M };
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)state);
        return 0;
    }
    case WM_COMMAND: {
		AppState* state = (AppState*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        switch (LOWORD(wParam)) {
        case ID_FILE_EXIT: {
            DestroyWindow(hWnd);
            return 0;
        }
        case ID_FILE_CLEAR: {
            for (int y = 0; y < M; y++) {
                for (int x = 0; x < N; x++) {
                    state->pixels[y][x] = 0;
                }
            }
            state->hasPrevPt = false;
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }
        case ID_MODE_FOLLOWING: {
            CheckMenuItem(GetMenu(hWnd), ID_MODE_FOLLOWING, MF_CHECKED);
            CheckMenuItem(GetMenu(hWnd), ID_MODE_LINE, MF_UNCHECKED);
            state->lineMode = false;
            return 0;
        }
        case ID_MODE_LINE: {
            CheckMenuItem(GetMenu(hWnd), ID_MODE_FOLLOWING, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), ID_MODE_LINE, MF_CHECKED);
            state->lineMode = true;
            return 0;
        }
        }
        return 0;
    }
    case WM_SIZING: {
		WORD fwSide = LOWORD(wParam);
		LPRECT pRect = (LPRECT)lParam;

		const double minWidth = 1.2 * N;
		const double minHeight = 1.2 * M;

		int width = pRect->right - pRect->left;
		int height = pRect->bottom - pRect->top;

        if (width < minWidth) {
            if (fwSide == WMSZ_LEFT || fwSide == WMSZ_TOPLEFT || fwSide == WMSZ_BOTTOMLEFT) {
                pRect->left = pRect->right - (int)minWidth;
            }
            else { pRect->right = pRect->left + (int)minWidth; }
        }

        if (height < minHeight) {
            if (fwSide == WMSZ_TOP || fwSide == WMSZ_TOPLEFT || fwSide == WMSZ_TOPRIGHT) {
                pRect->top = pRect->bottom - (int)minHeight;
            }
            else { pRect->bottom = pRect->top + (int)minHeight; }
		}

        return 0;
    }
    case WM_PAINT: {
        AppState* state = (AppState*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
        Rectangle(hdc, state->drawRect.left, state->drawRect.top, state->drawRect.right, state->drawRect.bottom);
        for (int y = 0; y < M; y++) {
            for (int x = 0; x < N; x++) {
                if (state->pixels[y][x]) {
                    SetPixelV(hdc, x, y, RGB(0, 0, 0));
                }
			}
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_MOUSEMOVE: {
        AppState* state = (AppState*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        POINT mPt = { LOWORD(lParam), HIWORD(lParam) };
        if (!state->lineMode) {
            if (wParam & MK_LBUTTON)
            {
                Draw(hWnd, state, mPt);
            }
            else {
                state->hasPrevPt = false;
            }
        }
        
        return 0;
    }
    case WM_LBUTTONDOWN: {
        AppState* state = (AppState*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        POINT mPt = { LOWORD(lParam), HIWORD(lParam) };
        if (state->lineMode) {
            Draw(hWnd, state, mPt);
        }
        return 0;
    }
    case WM_DESTROY:
		AppState* state = (AppState*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        delete state;
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}
//=========================================================
