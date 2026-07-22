#include <windows.h>
#include <string>

using namespace std;

struct InventoryData {
    string computerName;
    string cpu;
    string ram;
    string diskSize;
    string osVersion;
};

string GetComputerNameX() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    GetComputerNameA(buffer, &size);
    return string(buffer);
}

string GetOSVersion() {
    HKEY hKey;
    char os[256];
    DWORD size = sizeof(os);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)os, &size);
        RegCloseKey(hKey);
        return string(os);
    }
    return "Unknown";
}

string GetCPU() {
    HKEY hKey;
    char cpu[256];
    DWORD size = sizeof(cpu);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpu, &size);
        RegCloseKey(hKey);
        return string(cpu);
    }
    return "Unknown";
}

string GetRAM() {
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    return to_string(mem.ullTotalPhys / (1024 * 1024 * 1024)) + " GB";
}

string GetDiskSize() {
    ULARGE_INTEGER total;
    GetDiskFreeSpaceExA("C:\\", NULL, &total, NULL);
    return to_string(total.QuadPart / (1024 * 1024 * 1024)) + " GB";
}

InventoryData CollectData() {
    InventoryData data;
    data.computerName = GetComputerNameX();
    data.cpu = GetCPU();
    data.ram = GetRAM();
    data.diskSize = GetDiskSize();
    data.osVersion = GetOSVersion();
    return data;
}

HWND hBtnCollect, hBtnSave, hBtnSend, hText, hStatus, hTitle;
InventoryData currentData;
HFONT hFont, hFontBtn, hFontTitle, hFontStatus;
HBRUSH hBgBrush, hBtnBrush, hWhiteBrush;
WNDPROC oldEditProc;

#define BTN_COLLECT 101
#define BTN_SAVE 102
#define BTN_SEND 104
#define TEXT_AREA 103
#define STATUS_BAR 105
#define TITLE_TEXT 106
#define COLOR_BG      RGB(28, 28, 36)
#define COLOR_BTN     RGB(88, 101, 242)
#define COLOR_DIM     RGB(140, 140, 155)

LRESULT CALLBACK EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_ERASEBKGND) {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH br = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &rc, br);
        DeleteObject(br);
        return 1;
    }
    return CallWindowProcA(oldEditProc, hwnd, uMsg, wParam, lParam);
}

void ResizeControls(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    int btnWidth = 180;
    int btnHeight = 50;
    int gap = 20;
    int totalBtns = btnWidth * 3 + gap * 2;
    int startX = (w - totalBtns) / 2;
    int btnY = 120;

    SetWindowPos(hTitle, NULL, 0, 30, w, 40, SWP_NOZORDER);
    SetWindowPos(hBtnCollect, NULL, startX, btnY, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(hBtnSave, NULL, startX + btnWidth + gap, btnY, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(hBtnSend, NULL, startX + (btnWidth + gap) * 2, btnY, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(hText, NULL, 40, 200, w - 80, h - 290, SWP_NOZORDER);
    SetWindowPos(hStatus, NULL, 0, h - 32, w, 32, SWP_NOZORDER);
}

void UpdateText(string text) {
    SetWindowTextA(hText, text.c_str());
}

void SetStatus(string text) {
    SetWindowTextA(hStatus, ("  " + text).c_str());
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {

        hBgBrush = CreateSolidBrush(COLOR_BG);
        hBtnBrush = CreateSolidBrush(COLOR_BTN);
        hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));

        hFontTitle = CreateFontA(28, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

        hFontBtn = CreateFontA(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

        hFont = CreateFontA(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");

        hFontStatus = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

        hTitle = CreateWindowA(
            "STATIC", "Inventory Agent",
            WS_VISIBLE | WS_CHILD | SS_CENTER,
            0, 0, 0, 0, hwnd, (HMENU)TITLE_TEXT, NULL, NULL
        );

        hBtnCollect = CreateWindowA(
            "BUTTON", "  Collect Data",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            0, 0, 0, 0, hwnd, (HMENU)BTN_COLLECT, NULL, NULL
        );

        hBtnSave = CreateWindowA(
            "BUTTON", "  Save to File",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            0, 0, 0, 0, hwnd, (HMENU)BTN_SAVE, NULL, NULL
        );

        hBtnSend = CreateWindowA(
            "BUTTON", "  Send to 1C",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            0, 0, 0, 0, hwnd, (HMENU)BTN_SEND, NULL, NULL
        );

        hText = CreateWindowA(
            "EDIT", "",
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_BORDER,
            0, 0, 0, 0, hwnd, (HMENU)TEXT_AREA, NULL, NULL
        );

        oldEditProc = (WNDPROC)SetWindowLongPtrA(hText, GWLP_WNDPROC, (LONG_PTR)EditProc);

        hStatus = CreateWindowA(
            "STATIC", "  Ready",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            0, 0, 0, 0, hwnd, (HMENU)STATUS_BAR, NULL, NULL
        );

        SendMessageA(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
        SendMessageA(hBtnCollect, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
        SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
        SendMessageA(hBtnSend, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
        SendMessageA(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFontStatus, TRUE);

        ResizeControls(hwnd);
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND ctrl = (HWND)lParam;
        SetBkMode(hdc, TRANSPARENT);

        if (ctrl == hTitle) {
            SetTextColor(hdc, RGB(200, 210, 255));
            return (LRESULT)hBgBrush;
        }
        if (ctrl == hStatus) {
            SetBkColor(hdc, RGB(40, 40, 52));
            SetTextColor(hdc, COLOR_DIM);
            HBRUSH br = CreateSolidBrush(RGB(40, 40, 52));
            return (LRESULT)br;
        }
        return (LRESULT)hBgBrush;
    }

    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, COLOR_BTN);
        SetTextColor(hdc, RGB(255, 255, 255));
        return (LRESULT)hBtnBrush;
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hBgBrush);
        return 1;
    }

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
        if (dis->CtlType == ODT_BUTTON) {
            FillRect(dis->hDC, &dis->rcItem, hBtnBrush);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, RGB(255, 255, 255));

            char text[64];
            GetWindowTextA(dis->hwndItem, text, 64);

            RECT rc = dis->rcItem;
            rc.left += 20;
            DrawTextA(dis->hDC, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            if (dis->itemState & ODS_SELECTED) {
                HBRUSH br = CreateSolidBrush(RGB(70, 82, 200));
                FillRect(dis->hDC, &dis->rcItem, br);
                DrawTextA(dis->hDC, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                DeleteObject(br);
            }

            return TRUE;
        }
        return 0;
    }

    case WM_SIZE: {
        ResizeControls(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);

        if (id == BTN_COLLECT) {
            currentData = CollectData();

            string info = "  Computer:  " + currentData.computerName + "\r\n\r\n";
            info += "  OS:        " + currentData.osVersion + "\r\n\r\n";
            info += "  CPU:       " + currentData.cpu + "\r\n\r\n";
            info += "  RAM:       " + currentData.ram + "\r\n\r\n";
            info += "  Disk C:    " + currentData.diskSize;

            UpdateText(info);
            SetStatus("Data collected successfully");
        }

        if (id == BTN_SAVE) {
            if (currentData.computerName.empty()) {
                SetStatus("Collect data first");
                return 0;
            }

            string filename = currentData.computerName + "_inventory.txt";

            string content = "=== Inventory Report ===\r\n\r\n";
            content += "Computer: " + currentData.computerName + "\r\n";
            content += "OS: " + currentData.osVersion + "\r\n";
            content += "CPU: " + currentData.cpu + "\r\n";
            content += "RAM: " + currentData.ram + "\r\n";
            content += "Disk C: " + currentData.diskSize + "\r\n";

            HANDLE hFile = CreateFileA(
                filename.c_str(),
                GENERIC_WRITE,
                0, NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD written;
                WriteFile(hFile, content.c_str(), content.length(), &written, NULL);
                CloseHandle(hFile);
                SetStatus("Saved to " + filename);
            }
            else {
                SetStatus("Failed to save file");
            }
        }

        if (id == BTN_SEND) {
            if (currentData.computerName.empty()) {
                SetStatus("Collect data first");
                return 0;
            }

            SetStatus("Sending to 1C...");

            string cmd = "powershell -Command \"$conn = New-Object -ComObject V83.COMConnector; $conn.Connect('File=\"\"C:/Base/DataCenter\"\"'); $conn.Invoke('ИнвентаризацияСерверов.ОбновитьДанныеСервера', '" + currentData.computerName + "," + currentData.cpu + "," + currentData.ram + "," + currentData.diskSize + "," + currentData.osVersion + "'); $conn.Disconnect(); Write-Host 'OK'\"";

            system(cmd.c_str());
            SetStatus("Data sent to 1C");
        }

        return 0;
    }

    case WM_DESTROY:
        DeleteObject(hFont);
        DeleteObject(hFontBtn);
        DeleteObject(hFontTitle);
        DeleteObject(hFontStatus);
        DeleteObject(hBgBrush);
        DeleteObject(hBtnBrush);
        DeleteObject(hWhiteBrush);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "InvAgent";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowA(
        "InvAgent",
        "Inventory Agent",
        WS_OVERLAPPEDWINDOW,
        0, 0, width, height,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_MAXIMIZE);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}