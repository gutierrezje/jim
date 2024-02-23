#include "Renderer.h"

Renderer::Renderer(HWND windowHandle, Uint2 screenSize)
{
    mDevice = std::make_unique<Device>(windowHandle, screenSize);
    mGraphicsContext = mDevice->CreateGraphicsContext();
}

Renderer::~Renderer() {}

void Renderer::Render()
{
    RenderClearColor();
}

void Renderer::RenderClearColor()
{
    mDevice->BeginFrame();
    TextureResource& backBuffer = mDevice->GetCurrentBackBuffer();

    mGraphicsContext->Reset();

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mGraphicsContext->FlushBarriers();

    mGraphicsContext->ClearRenderTarget(backBuffer, Color(0.0, 0.018, 0.001));

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    mGraphicsContext->FlushBarriers();

    mDevice->SubmitContextWork(*mGraphicsContext);

    mDevice->EndFrame();
    mDevice->Present();
}