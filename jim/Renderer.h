#pragma once
#include "D3D12Lite.h"

using namespace D3D12Lite;

class Renderer
{
public:
    Renderer(HWND windowHandle, Uint2 screenSize);
    ~Renderer();
    void Render();

private:
    std::unique_ptr<Device> mDevice;
    std::unique_ptr<GraphicsContext> mGraphicsContext;

    void RenderClearColor();
};