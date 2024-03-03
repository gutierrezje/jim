#include "Renderer.h"

Renderer::Renderer(HWND windowHandle, Uint2 screenSize)
{
    mDevice = std::make_unique<Device>(windowHandle, screenSize);
    mGraphicsContext = mDevice->CreateGraphicsContext();

    initializeTriangleResources();
}

Renderer::~Renderer()
{
    mDevice->WaitForIdle();
    mDevice->DestroyPipelineStateObject(std::move(mTrianglePSO));
    mDevice->DestroyShader(std::move(mTriangleVertexShader));
    mDevice->DestroyShader(std::move(mTrianglePixelShader));
    mDevice->DestroyBuffer(std::move(mTriangleVertexBuffer));
    mDevice->DestroyBuffer(std::move(mTriangleConstantBuffer));
    mDevice->DestroyContext(std::move(mGraphicsContext));
    mDevice = nullptr;
    
}

void Renderer::Render()
{
    renderTriangle();
}

void Renderer::initializeTriangleResources()
{
    std::array<TriangleVertex, 3> vertices;
    vertices[0].position = { -0.5f, -0.5f };
    vertices[0].color = { 1.0f, 0.0f, 0.0f };
    vertices[1].position = { 0.0f, 0.5f };
    vertices[1].color = { 0.0f, 1.0f, 0.0f };
    vertices[2].position = { 0.5f, -0.5f };
    vertices[2].color = { 0.0f, 0.0f, 1.0f };

    BufferCreationDesc triangleBufferDesc{};
    triangleBufferDesc.mSize = sizeof(vertices);
    triangleBufferDesc.mAccessFlags = BufferAccessFlags::hostWritable;
    triangleBufferDesc.mViewFlags = BufferViewFlags::srv;
    triangleBufferDesc.mStride = sizeof(TriangleVertex);
    triangleBufferDesc.mIsRawAccess = true;

    mTriangleVertexBuffer = mDevice->CreateBuffer(triangleBufferDesc);
    mTriangleVertexBuffer->SetMappedData(&vertices, sizeof(vertices));

    BufferCreationDesc  triangleConstantDesc{};
    triangleConstantDesc.mSize = sizeof(TriangleConstants);
    triangleConstantDesc.mAccessFlags = BufferAccessFlags::hostWritable;
    triangleConstantDesc.mViewFlags = BufferViewFlags::cbv;

    TriangleConstants triangleConstants;
    triangleConstants.vertexBufferIndex = mTriangleVertexBuffer->mDescriptorHeapIndex;

    mTriangleConstantBuffer = mDevice->CreateBuffer(triangleConstantDesc);
    mTriangleConstantBuffer->SetMappedData(&triangleConstants, sizeof(TriangleConstants));

    ShaderCreationDesc triangleShaderVSDesc;
    triangleShaderVSDesc.mShaderName = L"Triangle.hlsl";
    triangleShaderVSDesc.mEntryPoint = L"VertexShader";
    triangleShaderVSDesc.mType = ShaderType::vertex;

    ShaderCreationDesc triangleShaderPSDesc;
    triangleShaderPSDesc.mShaderName = L"Triangle.hlsl";
    triangleShaderPSDesc.mEntryPoint = L"PixelShader";
    triangleShaderPSDesc.mType = ShaderType::pixel;

    mTriangleVertexShader = mDevice->CreateShader(triangleShaderVSDesc);
    mTrianglePixelShader = mDevice->CreateShader(triangleShaderPSDesc);

    mTrianglePerObjectSpace.SetCBV(mTriangleConstantBuffer.get());
    mTrianglePerObjectSpace.Lock();

    PipelineResourceLayout resourceLayout;
    resourceLayout.mSpaces[PER_OBJECT_SPACE] = &mTrianglePerObjectSpace;

    GraphicsPipelineDesc trianglePipelineDesc = GetDefaultGraphicsPipelineDesc();
    trianglePipelineDesc.mVertexShader = mTriangleVertexShader.get();
    trianglePipelineDesc.mPixelShader = mTrianglePixelShader.get();
    trianglePipelineDesc.mRenderTargetDesc.mNumRenderTargets = 1;
    trianglePipelineDesc.mRenderTargetDesc.mRenderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    mTrianglePSO = mDevice->CreateGraphicsPipeline(trianglePipelineDesc, resourceLayout);
}

void Renderer::renderTriangle()
{
    mDevice->BeginFrame();

    TextureResource& backBuffer = mDevice->GetCurrentBackBuffer();

    mGraphicsContext->Reset();
    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mGraphicsContext->FlushBarriers();

    mGraphicsContext->ClearRenderTarget(backBuffer, Color(0.0, 0.018, 0.001));

    PipelineInfo pipeline;
    pipeline.mPipeline = mTrianglePSO.get();
    pipeline.mRenderTargets.push_back(&backBuffer);

    mGraphicsContext->SetPipeline(pipeline);
    mGraphicsContext->SetPipelineResources(PER_OBJECT_SPACE, mTrianglePerObjectSpace);
    mGraphicsContext->SetDefaultViewPortAndScissor(mDevice->GetScreenSize());
    mGraphicsContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mGraphicsContext->Draw(3);

    mGraphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    mGraphicsContext->FlushBarriers();

    mDevice->SubmitContextWork(*mGraphicsContext);

    mDevice->EndFrame();
    mDevice->Present();
}
