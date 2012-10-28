#include <windows.h>
#include <stdio.h>

// Author: Vasiliy Poverennov <bazookavn@gmail.com>
// https://bitbucket.org/bazookavrn/relocator

#define NUM_RETRIES 10
#define SLEEP_TIME 15000

HWND app_window = 0;
BOOL CALLBACK find_app(HWND hwnd, LPARAM look_for) {
    DWORD procid;
    GetWindowThreadProcessId(hwnd, &procid);
    if ((DWORD)look_for == procid) {
        app_window = hwnd;
        return FALSE; // finish search
    } else {
        return TRUE;
    }
}

int relocate(HWND hwnd) {
    if (!hwnd) {
        return 0;
    }
    RECT screen;
    if (!GetWindowRect(GetDesktopWindow(), &screen)) {
        return 0;
    }

    LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (lStyle == 0) {
        return 0;
    }
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(hwnd, GWL_STYLE, lStyle);
/* // not sure if its needed
    LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
*/

    if (!SetWindowPos(hwnd, NULL, screen.left, screen.top, 
                      screen.right - screen.left, screen.bottom - screen.top,
                      SWP_NOZORDER | SWP_FRAMECHANGED)) {
        return 0;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (argc != 2) {
        printf("usage: %s game\n", argv[0]);
        return 1;
    }

    if (!CreateProcess(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return 1;
    }

    if (WaitForInputIdle(pi.hProcess, INFINITE)) {
        return 1;
    }

    int i;
    for (i = 0; i < NUM_RETRIES && (app_window == 0); i++) {
        EnumWindows(find_app, (LPARAM)pi.dwProcessId);
        Sleep(SLEEP_TIME); // hack for 
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return !relocate(app_window);
}