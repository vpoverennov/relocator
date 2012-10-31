#define UNICODE 1
#define _UNICODE 1
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

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
    RECT screen;
    LONG lStyle;
    if (!hwnd) {
        return 0;
    }
    if (!GetWindowRect(GetDesktopWindow(), &screen)) {
        return 0;
    }

    lStyle = GetWindowLong(hwnd, GWL_STYLE);
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

//int main(int argc, char* argv[]) {
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    size_t i;

    LPWSTR *argv;
    int argc;
    LPWSTR game_cmd;
    size_t game_cmd_len = 0;
    LPTSTR cmdline = GetCommandLine();

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    argv = CommandLineToArgvW(cmdline, &argc);
    if (argv == NULL) {
        return 1;
    }
    if (argc < 2) {
        wprintf(L"usage: %s game [args ...]\n", argv[0]);
        return 1;
    }

    // count len of resulting cmdline
    for (i = 1; i < argc; i++) {
        game_cmd_len = game_cmd_len + wcslen(argv[i]) + 1;
    }
    //concatenate argv into one string
    game_cmd = (LPWSTR)calloc(game_cmd_len + 1, sizeof(WCHAR));
    for (i = 1; i < argc; i++) {
        wcscat(game_cmd, argv[i]);
        if (i < argc - 1) {
            wcscat(game_cmd, L" ");
        }
    }
    LocalFree(argv);

    if (!CreateProcess(NULL, game_cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return 1;
    }
    free(game_cmd);

    if (WaitForInputIdle(pi.hProcess, INFINITE)) {
        return 1;
    }

    for (i = 0; i < NUM_RETRIES && (app_window == 0); i++) {
        EnumWindows(find_app, (LPARAM)pi.dwProcessId);
        Sleep(SLEEP_TIME); // hack for 
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return !relocate(app_window);
}