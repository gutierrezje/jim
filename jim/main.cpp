#include <fstream>
#include <iostream>
#include <cstddef>
#include <bitset>
#include <cstdint>

#include "Renderer.h"
#include "TTFReader.h"

// https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        else
        {
            return DefWindowProc(hwnd, umessage, wparam, lparam);
        }

    case WM_DESTROY:
        [[fallthrough]];
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, umessage, wparam, lparam);
    }
}

class TrueTypeFont
{
public:
    TrueTypeFont(const std::string& ttf_file);

private:

    TTFReader mTTFReader;
    TTF::TableDirectory mTableDirectory;
    std::unordered_map<std::wstring, TTF::TableRecord> mTables;
    TTF::HeadTable mHeadTable;
    uint32_t mGlyphOffset;
    int length;
};

TrueTypeFont::TrueTypeFont(const std::string& ttf_file) : mTTFReader{ ttf_file }
{
    mTableDirectory = mTTFReader.readTableDirectory();
    
    for (int i = 0; i < mTableDirectory.numTables; i += 1) {
        auto nextRecord = mTTFReader.readTableRecord();
        mTables.emplace(nextRecord);
    }
    if (auto headRecord = mTables.find(L"head"); headRecord != mTables.end()) {
        mHeadTable = mTTFReader.readHeadTable(headRecord->second.offset);
    }
    else {
        std::wcerr << "head table not found in TTF file.\n";
    }
    if (auto locaRecord = mTables.find(L"loca"); locaRecord != mTables.end()) {
        mGlyphOffset = mTTFReader.readGlyphOffset(locaRecord->second.offset, 1, mHeadTable.indexToLocFormat);
    }
    else {
        std::wcerr << "loca table not found in TTF file.\n";
    }
    std::cout << "DONE\n";
}

int main()
{
    std::wstring applicationName = L"jim";
    Uint2 windowSize = { 1920, 1080 };
    HINSTANCE moduleHandle = GetModuleHandle(nullptr);

    WNDCLASSEX wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = moduleHandle;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = applicationName.c_str();
    wc.cbSize = sizeof(WNDCLASSEX);
    RegisterClassEx(&wc);

    HWND windowHandle = CreateWindowEx(WS_EX_APPWINDOW, applicationName.c_str(), applicationName.c_str(),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_SIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - windowSize.x) / 2, (GetSystemMetrics(SM_CYSCREEN) - windowSize.y) / 2,
        windowSize.x, windowSize.y, nullptr, nullptr, moduleHandle, nullptr);

    ShowWindow(windowHandle, SW_SHOW);
    SetForegroundWindow(windowHandle);
    SetFocus(windowHandle);
    ShowCursor(true);

    auto renderer = std::make_unique<Renderer>(windowHandle, windowSize);
    
    TrueTypeFont ttf("res/Fonts/CascadiaMono.ttf");

    bool shouldExit = false;
    while (!shouldExit) {
        MSG msg{ 0 };
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) {
            shouldExit = true;
        }

        renderer->Render();
    }

    renderer.release();

    DestroyWindow(windowHandle);
    windowHandle = nullptr;

    UnregisterClass(applicationName.c_str(), moduleHandle);
    moduleHandle = nullptr;

    return 0;
}