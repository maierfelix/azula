#include "GPUContextD3D11.h"
#include <cassert>

namespace ultralight {

  D3D_DRIVER_TYPE driverTypes[] =
  {
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_REFERENCE,
    D3D_DRIVER_TYPE_WARP,
  };

  D3D_FEATURE_LEVEL featureLevels[] =
  {
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
  };

  GPUContextD3D11::GPUContextD3D11() {}

  GPUContextD3D11::~GPUContextD3D11() {
    if (device_)
      immediate_context_->ClearState();

#if defined(_DEBUG)
    ID3D11Debug* debug;
    HRESULT result = device_.Get()->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void **>(&debug));

    if (SUCCEEDED(result))
    {
      debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
      debug->Release();
    }
#endif
  }

  // Inherited from GPUContext
  void GPUContextD3D11::BeginDrawing() {}
  void GPUContextD3D11::EndDrawing() {}
  void GPUContextD3D11::PresentFrame() {
    swap_chain()->Present(enable_vsync_ ? 1 : 0, 0);
  }

  void GPUContextD3D11::Resize(int width, int height) {
    set_screen_size(width, height);

    immediate_context_->OMSetRenderTargets(0, 0, 0);
    ID3D11Texture2D* back_buffer = nullptr;
    swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer);
    back_buffer->Release();

    back_buffer_view_.Reset();

    // Get the actual device width/height (may be different than screen size)
    RECT rc;
    ::GetClientRect(hwnd_, &rc);
    UINT client_width = rc.right - rc.left;
    UINT client_height = rc.bottom - rc.top;

    HRESULT hr;
    hr = swap_chain_->ResizeBuffers(0, client_width, client_height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
      MessageBoxW(nullptr,
        L"GPUContextD3D11::Resize, unable to resize, IDXGISwapChain::ResizeBuffers failed.", L"Error", MB_OK);
      exit(-1);
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
      MessageBoxW(nullptr,
        L"GPUContextD3D11::Resize, unable to get back buffer.", L"Error", MB_OK);
      exit(-1);
    }

    hr = device_->CreateRenderTargetView(pBackBuffer, nullptr, back_buffer_view_.GetAddressOf());
    pBackBuffer->Release();
    if (FAILED(hr))
    {
      MessageBoxW(nullptr,
        L"GPUContextD3D11::Resize, unable to create new render target view.", L"Error", MB_OK);
      exit(-1);
    }

    immediate_context_->OMSetRenderTargets(1, back_buffer_view_.GetAddressOf(), nullptr);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = (float)client_width * (float)scale();
    vp.Height = (float)client_height * (float)scale();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    immediate_context_->RSSetViewports(1, &vp);
  }

  ID3D11Device* GPUContextD3D11::device() { assert(device_.Get()); return device_.Get(); }
  ID3D11DeviceContext* GPUContextD3D11::immediate_context() { assert(immediate_context_.Get()); return immediate_context_.Get(); }
  IDXGISwapChain* GPUContextD3D11::swap_chain() { assert(swap_chain_.Get()); return swap_chain_.Get(); }
  ID3D11RenderTargetView* GPUContextD3D11::render_target_view() { assert(back_buffer_view_.Get()); return back_buffer_view_.Get(); }

  void GPUContextD3D11::EnableBlend() {
    immediate_context_->OMSetBlendState(blend_state_.Get(), NULL, 0xffffffff);
  }

  void GPUContextD3D11::DisableBlend() {
    immediate_context_->OMSetBlendState(disabled_blend_state_.Get(), NULL, 0xffffffff);
  }

  void GPUContextD3D11::EnableScissor() {
    immediate_context_->RSSetState(scissored_rasterizer_state_.Get());
  }

  void GPUContextD3D11::DisableScissor() {
    immediate_context_->RSSetState(rasterizer_state_.Get());
  }

  // Scale is calculated from monitor DPI, see Application::SetScale
  void GPUContextD3D11::set_scale(double scale) { scale_ = scale; }
  double GPUContextD3D11::scale() const { return scale_; }

  // This is in units, not actual pixels.
  void GPUContextD3D11::set_screen_size(uint32_t width, uint32_t height) { width_ = width; height_ = height; }
  void GPUContextD3D11::screen_size(uint32_t& width, uint32_t& height) { width = width_; height = height_; }

  bool GPUContextD3D11::SetupDevice() {
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    for (unsigned driverTypeIndex = 0; driverTypeIndex < ARRAYSIZE(driverTypes); driverTypeIndex++)
    {
      D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
      hr = D3D11CreateDevice(
        nullptr,
        driverType,
        0, createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, device_.GetAddressOf(), &feature_level_, immediate_context_.GetAddressOf()
      );
      if (SUCCEEDED(hr)) break;
    }
    if (FAILED(hr))
    {
      MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Unable to create device.", (LPCWSTR)L"D3D11 Error", MB_OK);
      // Unable to create device
      return false;
    }

    return true;
  }

  bool GPUContextD3D11::SetupDeviceAndSwapChain(HWND hWnd, int width, int height, bool fullscreen, bool sRGB) {
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DXGI_SWAP_CHAIN_DESC sd;
    ::ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    //sd.SampleDesc.Count = samples_;
    //sd.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = !fullscreen;
    sd.Flags = fullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

    for (unsigned driverTypeIndex = 0; driverTypeIndex < ARRAYSIZE(driverTypes); driverTypeIndex++)
    {
      D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
      hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, &sd, swap_chain_.GetAddressOf(), device_.GetAddressOf(), &feature_level_, immediate_context_.GetAddressOf());
      if (SUCCEEDED(hr))
      {
        break;
      }
    }
    if (FAILED(hr))
    {
      MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Unable to create device and swap chain.", (LPCWSTR)L"D3D11 Error", MB_OK);
      // Unable to create device  & swap chain
      return false;
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
      MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Unable to get back buffer.", (LPCWSTR)L"D3D11 Error", MB_OK);
      // Unable get back buffer
      return false;
    }

    hr = device_->CreateRenderTargetView(pBackBuffer, nullptr, back_buffer_view_.GetAddressOf());
    pBackBuffer->Release();
    if (FAILED(hr))
    {
      MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Unable to create RenderTargetView.", (LPCWSTR)L"D3D11 Error", MB_OK);
      // Unable create back buffer view
      return false;
    }

    immediate_context_->OMSetRenderTargets(1, back_buffer_view_.GetAddressOf(), nullptr);

    return true;
  }

  bool GPUContextD3D11::Initialize(HWND hWnd, int screen_width, int screen_height, double screen_scale, bool fullscreen, bool enable_vsync, bool sRGB, int samples) {
    samples_ = samples;
#if ENABLE_MSAA
    samples_ = 4;
#endif
    enable_vsync_ = enable_vsync;
    set_screen_size(screen_width, screen_height);
    set_scale(screen_scale);

    if (hWnd != NULL) hwnd_ = hWnd;

    // Get the actual device width/height (may be different than screen size)
    RECT rc;
    if (hWnd != NULL) 
    {
      ::GetClientRect(hWnd, &rc);
    }
    else
    {
      rc.left = 0;
      rc.right = screen_width;
      rc.top = 0;
      rc.bottom = screen_height;
    }
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    if (hWnd == NULL)
    {
      if (!SetupDevice())
      {
        MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Failed to create device.", (LPCWSTR)L"D3D11 Error", MB_OK);
        return false;
      }
    }
    else
    {
      if (!SetupDeviceAndSwapChain(hWnd, width, height, fullscreen, sRGB))
      {
        MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Failed to create device and swapchain.", (LPCWSTR)L"D3D11 Error", MB_OK);
        return false;
      }
    }

    // Create Enabled Blend State

    D3D11_RENDER_TARGET_BLEND_DESC rt_blend_desc;
    ZeroMemory(&rt_blend_desc, sizeof(rt_blend_desc));
    rt_blend_desc.BlendEnable = true;
    rt_blend_desc.SrcBlend = D3D11_BLEND_ONE;
    rt_blend_desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rt_blend_desc.BlendOp = D3D11_BLEND_OP_ADD;
    rt_blend_desc.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
    rt_blend_desc.DestBlendAlpha = D3D11_BLEND_ONE;
    rt_blend_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC blend_desc;
    ZeroMemory(&blend_desc, sizeof(blend_desc));
    blend_desc.AlphaToCoverageEnable = false;
    blend_desc.IndependentBlendEnable = false;
    blend_desc.RenderTarget[0] = rt_blend_desc;

    device()->CreateBlendState(&blend_desc, blend_state_.GetAddressOf());

    // Create Disabled Blend State

    ZeroMemory(&rt_blend_desc, sizeof(rt_blend_desc));
    rt_blend_desc.BlendEnable = false;
    rt_blend_desc.SrcBlend = D3D11_BLEND_ONE;
    rt_blend_desc.DestBlend = D3D11_BLEND_ZERO;
    rt_blend_desc.BlendOp = D3D11_BLEND_OP_ADD;
    rt_blend_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rt_blend_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
    rt_blend_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ZeroMemory(&blend_desc, sizeof(blend_desc));
    blend_desc.AlphaToCoverageEnable = false;
    blend_desc.IndependentBlendEnable = false;
    blend_desc.RenderTarget[0] = rt_blend_desc;

    device()->CreateBlendState(&blend_desc, disabled_blend_state_.GetAddressOf());

    EnableBlend();

    D3D11_RASTERIZER_DESC rasterizer_desc;
    ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_NONE;
    rasterizer_desc.FrontCounterClockwise = false;
    rasterizer_desc.DepthBias = 0;
    rasterizer_desc.SlopeScaledDepthBias = 0.0f;
    rasterizer_desc.DepthBiasClamp = 0.0f;
    rasterizer_desc.DepthClipEnable = false;
    rasterizer_desc.ScissorEnable = false;
#if ENABLE_MSAA
    rasterizer_desc.MultisampleEnable = true;
    rasterizer_desc.AntialiasedLineEnable = true;
#else
    rasterizer_desc.MultisampleEnable = false;
    rasterizer_desc.AntialiasedLineEnable = false;
#endif

    device()->CreateRasterizerState(&rasterizer_desc, rasterizer_state_.GetAddressOf());

    D3D11_RASTERIZER_DESC scissor_rasterizer_desc;
    ZeroMemory(&scissor_rasterizer_desc, sizeof(scissor_rasterizer_desc));
    scissor_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    scissor_rasterizer_desc.CullMode = D3D11_CULL_NONE;
    scissor_rasterizer_desc.FrontCounterClockwise = false;
    scissor_rasterizer_desc.DepthBias = 0;
    scissor_rasterizer_desc.SlopeScaledDepthBias = 0.0f;
    scissor_rasterizer_desc.DepthBiasClamp = 0.0f;
    scissor_rasterizer_desc.DepthClipEnable = false;
    scissor_rasterizer_desc.ScissorEnable = true;
#if ENABLE_MSAA
    scissor_rasterizer_desc.MultisampleEnable = true;
    scissor_rasterizer_desc.AntialiasedLineEnable = true;
#else
    scissor_rasterizer_desc.MultisampleEnable = false;
    scissor_rasterizer_desc.AntialiasedLineEnable = false;
#endif

    device()->CreateRasterizerState(&scissor_rasterizer_desc, scissored_rasterizer_state_.GetAddressOf());

    DisableScissor();

    // Setup the viewport
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = (float)width * (float)scale();
    vp.Height = (float)height * (float)scale();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    immediate_context_->RSSetViewports(1, &vp);

    // Initialize backbuffer with white so we don't get flash of black while loading views.
    if (hWnd != NULL)
    {
      back_buffer_width_ = width;
      back_buffer_height_ = height;

      float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
      immediate_context_->ClearRenderTargetView(render_target_view(), color);
    }

    return true;
  }

  UINT GPUContextD3D11::back_buffer_width() { return back_buffer_width_; }
  UINT GPUContextD3D11::back_buffer_height() { return back_buffer_height_; }

}  // namespace ultralight
