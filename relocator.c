#define UNICODE 1
#define _UNICODE 1
#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

// Author: Vasiliy Poverennov <vasiliy@poverennov.com>
// https://bitbucket.org/bazookavrn/relocator

#define NUM_CHILDREN 10
#define NUM_RETRIES 120
#define SLEEP_TIME 500

static HWND app_window;
static DWORD app_pid;
static DWORD children[NUM_CHILDREN];

BOOL CALLBACK find_app(HWND hwnd, LPARAM param) {
    LONG win_style;
    DWORD procid;
    BOOL matches;
    size_t i;

    GetWindowThreadProcessId(hwnd, &procid);
    if (procid != app_pid) {
        matches = FALSE;
        for (i = 0; children[i]; i++) {
            if (procid == children[i]) {
                matches = TRUE;
                break;
            }
        }
        if (!matches) {
            return TRUE; // continue
        }
    }

    if (!IsWindowVisible(hwnd)) {
        wprintf(L"found: [%d] invisible\n", hwnd);
        return TRUE;
    }

    win_style = GetWindowLong(hwnd, GWL_STYLE);
    if (!(win_style & WS_CAPTION)) {
        wprintf(L"found: [%d] visible nocaption\n", hwnd);
        return TRUE;
    }
    wprintf(L"found: [%d] visible ok\n", hwnd);
    app_window = hwnd;
    return FALSE;
}


BOOL find_children(DWORD parent_pid) {
    HANDLE hProcessSnap;
    size_t current = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return FALSE;
    }

    do {
        if (pe32.th32ParentProcessID == parent_pid) {
            children[current] = pe32.th32ProcessID;
            current++;
        }
    } while (Process32Next(hProcessSnap, &pe32) && current < NUM_CHILDREN - 1);

    children[current] = 0; // null terminate list of children
    CloseHandle(hProcessSnap);
    return TRUE;
}

int relocate(HWND hwnd) {
    RECT screen;
    LONG win_style;
    if (!hwnd) {
        return 0;
    }
    if (!GetWindowRect(GetDesktopWindow(), &screen)) {
        return 0;
    }

    win_style = GetWindowLong(hwnd, GWL_STYLE);
    if (win_style == 0) {
        return 0;
    }
    win_style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(hwnd, GWL_STYLE, win_style);
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
    app_pid = pi.dwProcessId;
    wprintf(L"game started... [%d]\n", app_pid);

    if (WaitForInputIdle(pi.hProcess, INFINITE)) {
        return 1;
    }

    for (i = 0; i < NUM_RETRIES; i++) {
        wprintf(L"looking for windows...\n");
        if (find_children(pi.dwProcessId)) {
            EnumWindows(find_app, (LPARAM)pi.dwProcessId);
        }
        if (app_window != 0) {
            break;
        }
        wprintf(L"sleeping...\n");
        Sleep(SLEEP_TIME);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return !relocate(app_window);
}