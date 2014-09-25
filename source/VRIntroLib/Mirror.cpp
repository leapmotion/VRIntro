#include "stdafx.h"
#include "Mirror.h"

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <dwmapi.h>
#include <iostream>

HTHUMBNAIL g_hthumb;

/*
 *  OnCreate
 *      Applications will typically override this and maybe even
 *      create a child window.
 */
BOOL
OnCreate(HWND hwnd, LPCREATESTRUCT lpcs) {
  DWM_THUMBNAIL_PROPERTIES props = {};
  HWND hwndTarget = *reinterpret_cast<HWND*>(lpcs->lpCreateParams);

  props.rcSource.left = 0;
  props.rcSource.top = 0;
  props.rcSource.right = 1920;
  props.rcSource.bottom = 1080;
  
  // hwndTarget = (HWND)0x00060778;
  DwmRegisterThumbnail(hwnd, hwndTarget, &g_hthumb);
  props.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_RECTSOURCE |
                  DWM_TNP_RECTDESTINATION;
  props.rcDestination = props.rcSource;
  OffsetRect(&props.rcSource,
             -props.rcSource.left, -props.rcSource.top);
  props.fVisible = TRUE;
  DwmUpdateThumbnailProperties(g_hthumb, &props);
  return TRUE;
}

/*
 *  OnDestroy
 *      Post a quit message because our application is over when the
 *      user closes this window.
 */
void
OnDestroy(HWND hwnd) {
  if (g_hthumb) { DwmUnregisterThumbnail(g_hthumb); }
  PostQuitMessage(0);
}

/*
 *  PaintContent
 *      Interesting things will be painted here eventually.
 */
void
PaintContent(HWND hwnd, PAINTSTRUCT *pps) {
}

/*
 *  OnPaint
 *      Paint the content as part of the paint cycle.
 */
void
OnPaint(HWND hwnd) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd, &ps);
  PaintContent(hwnd, &ps);
  EndPaint(hwnd, &ps);
}

/*
 *  OnPrintClient
 *      Paint the content as requested by USER.
 */
void
OnPrintClient(HWND hwnd, HDC hdc) {
  PAINTSTRUCT ps;
  ps.hdc = hdc;
  GetClientRect(hwnd, &ps.rcPaint);
  PaintContent(hwnd, &ps);

}

/*
 *  Window procedure
 */
LRESULT CALLBACK
WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
  switch (uiMsg) {

    HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
    HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
  case WM_PRINTCLIENT:
    OnPrintClient(hwnd, (HDC)wParam);
    return 0;
  }

  return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

BOOL
InitApp(void) {
  WNDCLASS wc;

  wc.style = 0;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
  wc.hIcon = NULL;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = TEXT("Mirror");

  if (!RegisterClass(&wc)) { return FALSE; }

  InitCommonControls();               /* In case we use a common control */

  return TRUE;
}

void RunMirror(HWND parentHwnd, HWND& outHwnd) {
  MSG msg;

  InitApp();

  if (SUCCEEDED(CoInitialize(NULL))) {/* In case we use COM */

    outHwnd = CreateWindow(
             TEXT("Mirror"),                /* Class Name */
             TEXT("Mirror"),                /* Title */
             WS_OVERLAPPEDWINDOW,            /* Style */
             CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
             CW_USEDEFAULT, CW_USEDEFAULT,   /* Size */
             NULL,                           /* Parent */
             NULL,                           /* No menu */
             (HINSTANCE)GetModuleHandle(NULL),/* Instance */
             &parentHwnd);

    ShowWindow(outHwnd, SW_MAXIMIZE);

    while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    CoUninitialize();
  }
}
