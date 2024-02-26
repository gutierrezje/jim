#pragma once
#include "D3D12Lite.h"
#include "Shaders/Shared.h"

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
    std::unique_ptr<BufferResource> mTriangleVertexBuffer;
    std::unique_ptr<BufferResource> mTriangleConstantBuffer;
    std::unique_ptr<Shader> mTriangleVertexShader;
    std::unique_ptr<Shader> mTrianglePixelShader;
    std::unique_ptr<PipelineStateObject> mTrianglePSO;
    PipelineResourceSpace mTrianglePerObjectSpace;

    void renderTriangle();
    void initializeTriangleResources();
};