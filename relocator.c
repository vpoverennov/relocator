#include <windows.h>
#include <stdio.h>

// Author: Vasiliy Poverennov <bazookavn@gmail.com>

#define NUM_RETRIES 10
#define SLEEP_TIME 15000

HWND app_window = 0;
BOOL CALLBACK find_app(HWND hwnd, LPARAM llook_for) {
    DWORD look_for = (DWORD)llook_for;
    DWORD procid;
    GetWindowThreadProcessId(hwnd, &procid);
    if (look_for == procid) {
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
    RECT screen_rect;
    GetWindowRect(GetDesktopWindow(), &screen_rect);

    LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (lStyle == 0) {
        printf("fail get\n");
        return 0;
    }
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(hwnd, GWL_STYLE, lStyle);
/* // not sure if its needed
    LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
*/

    if (!MoveWindow(hwnd, screen_rect.left, screen_rect.top,
                    screen_rect.left + screen_rect.right,
                    screen_rect.top + screen_rect.bottom, TRUE)) {
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

    if (!CreateProcess(NULL,   // No module name (use command line)
        argv[1],        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
    ){
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