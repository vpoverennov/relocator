/* Author: Vasiliy Poverennov <vasiliy@poverennov.com>
 * https://github.com/vpoverennov/relocator
 */

#include <stdio.h>
#include <wctype.h>
#include <windows.h>
#include <tlhelp32.h>

#define MAX_CHILDREN 10
#define NUM_RETRIES 120
#define SLEEP_TIME 500

#define len(X) (sizeof(X) / sizeof(X[0]))
static wchar_t *wcsnlower(size_t count, wchar_t *in);
static BOOL find_children(DWORD parent_pid);
static BOOL CALLBACK find_app(HWND hwnd, LPARAM param);
static BOOL make_borderless(HWND window);
static BOOL relocate(HWND window);
static int process(size_t argc, wchar_t *argv[]);

static HWND game_window;
static DWORD game_pid;
static wchar_t wot_name[] = L"worldoftanks";
static DWORD children[MAX_CHILDREN];
static size_t num_children;

wchar_t *wcsnlower(size_t count, wchar_t *in) {
    size_t i;
    wchar_t *out = (wchar_t *)calloc(count + 1, sizeof(wchar_t));
    if (out == NULL) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        out[i] = towlower(in[i]);
    }
    out[count] = L'\0';
    return out;
}

BOOL find_children(DWORD parent_pid) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    num_children = 0;

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
            children[num_children] = pe32.th32ProcessID;
            num_children++;
        }
    } while (Process32Next(hProcessSnap, &pe32) && num_children < MAX_CHILDREN - 1);

    CloseHandle(hProcessSnap);
    return TRUE;
}

BOOL CALLBACK find_app(HWND hwnd, LPARAM param) {
    UNREFERENCED_PARAMETER(param);

    LONG win_style;
    DWORD procid;
    size_t i;

    GetWindowThreadProcessId(hwnd, &procid);
    if (procid != game_pid) {
        for (i = 0; i < num_children; i++) {
            if (procid == children[i]) {
                break;
            }
        }
        if (i == num_children) {
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
    game_window = hwnd;
    return FALSE;
}

BOOL make_borderless(HWND window) {
    LONG win_style;
    if (!window) {
        return FALSE;
    }
    win_style = GetWindowLong(window, GWL_STYLE);
    if (win_style == 0) {
        return FALSE;
    }
    win_style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(window, GWL_STYLE, win_style);
/* // not sure if its needed
    LONG lExStyle = GetWindowLong(window, GWL_EXSTYLE);
    lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(window, GWL_EXSTYLE, lExStyle);
*/
    if (!SetWindowPos(window, NULL, 100, 100, 800, 600,
                      SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED)) {
        return FALSE;
    }
    return TRUE;
}
BOOL relocate(HWND window) {
    RECT screen;
    if (!window) {
        return FALSE;
    }
    if (!GetWindowRect(GetDesktopWindow(), &screen)) {
        return FALSE;
    }
    if (!SetWindowPos(window, NULL, screen.left, screen.top,
                      screen.right - screen.left, screen.bottom - screen.top,
                      SWP_NOZORDER)) {
        return FALSE;
    }

    return TRUE;
}

int process(size_t argc, wchar_t *argv[]) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    size_t i;

    size_t game_cmd_len = 0;
    wchar_t *game_cmd;
    wchar_t *game_name;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (argc < 2) {
        MessageBox(
            NULL,
            (LPCWSTR)L"Get instructions at \nhttps://github.com/vpoverennov/relocator",
            (LPCWSTR)L"Does not work this way",
            MB_ICONWARNING | MB_OK
        );
        return 1;
    }

    game_name = wcsnlower(wcslen(argv[1]), argv[1]);
    // count len of resulting cmdline
    for (i = 1; i < argc; i++) {
        game_cmd_len = game_cmd_len + wcslen(argv[i]) + 1;
    }
    //concatenate argv into one string
    game_cmd = (wchar_t *)calloc(game_cmd_len + 1, sizeof(wchar_t));
    if (game_cmd == NULL) {
        return 1;
    }
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
    game_pid = pi.dwProcessId;
    wprintf(L"game started... [%d]\n", game_pid);
    if (WaitForInputIdle(pi.hProcess, INFINITE)) {
        return 1;
    }

    for (i = 0; i < NUM_RETRIES; i++, Sleep(SLEEP_TIME)) {
        wprintf(L"looking for windows...\n");
        if (find_children(pi.dwProcessId)) {
            EnumWindows(find_app, 0);
        }
        if (game_window) {
            break;
        }
    }
    if (!make_borderless(game_window)) {
        return 1;
    }

    // game specific hacks
    if (wcsncmp(game_name, wot_name, len(wot_name) - 1) == 0) {
        // theres a small visual glitch if wot is relocated too fast
        Sleep(30000);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return !relocate(game_window);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    int argc;
    wchar_t **argv;
    wchar_t *cmdline = GetCommandLine();
    argv = CommandLineToArgvW(cmdline, &argc);
    if (argv == NULL) {
        return 1;
    }
    if (argc < 0) {
        return 1;
    }
    return process((size_t)argc, argv);
}
