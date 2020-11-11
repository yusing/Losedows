// Copyright 2020 yusing
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.

// see imgui/examples

#pragma once
#ifndef BACKEND_USE_D3D12
# define DIRECTINPUT_VERSION 0x0800
#endif
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <dinput.h>
#include <dxgi.h>
#include <tchar.h>
#include "ui.h"
#if defined(BACKEND_USE_D3D9)
# pragma comment(lib, "d3d9.lib")
# define BACKEND_INIT ImGui_ImplDX9_Init(g_pd3dDevice);
# define BACKEND_NEW_FRAME ImGui_ImplDX9_NewFrame
# define BACKEND_SHUTDOWN ImGui_ImplDX9_Shutdown
# include <imgui_impl_dx9.h>
# include <d3d9.h>
  static LPDIRECT3D9 g_pD3D = nullptr;
  static LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;
  static D3DPRESENT_PARAMETERS g_d3dpp = {};
#elif defined(BACKEND_USE_D3D10)
# pragma comment(lib, "d3d10.lib")
# define BACKEND_INIT ImGui_ImplDX10_Init(g_pd3dDevice);
# define BACKEND_NEW_FRAME ImGui_ImplDX10_NewFrame
# define BACKEND_SHUTDOWN ImGui_ImplDX10_Shutdown
# define TEXTURE2D ID3D10Texture2D
# include <imgui_impl_dx10.h>
# include <d3d10_1.h>
# include <d3d10.h>
static ID3D10Device*           g_pd3dDevice           = nullptr;
static IDXGISwapChain*         g_pSwapChain           = nullptr;
static ID3D10RenderTargetView* g_mainRenderTargetView = nullptr;
#elif defined(BACKEND_USE_D3D11)
# pragma comment(lib, "d3d11.lib")
# define BACKEND_INIT ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);;
# define BACKEND_NEW_FRAME ImGui_ImplDX11_NewFrame
# define BACKEND_SHUTDOWN ImGui_ImplDX11_Shutdown
# define TEXTURE2D ID3D11Texture2D
# include <imgui_impl_dx11.h>
# include <d3d11.h>
  static ID3D11Device* g_pd3dDevice = nullptr;
  static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
  static IDXGISwapChain* g_pSwapChain = nullptr;
  static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
#else // D3D12
# pragma comment(lib, "d3d12.lib")
# define BACKEND_INIT ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,\
DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,\
g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),\
g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
# define BACKEND_NEW_FRAME ImGui_ImplDX12_NewFrame
# define BACKEND_SHUTDOWN ImGui_ImplDX12_Shutdown
# include <imgui_impl_dx12.h>
# include <d3d12.h>
# include <dxgi1_4.h>
# ifdef _DEBUG
# define DX12_ENABLE_DEBUG_LAYER
# include <dxgidebug.h>
# pragma comment(lib, "dxguid.lib")
# endif

struct FrameContext {
  ID3D12CommandAllocator* CommandAllocator;
  UINT64 FenceValue;
};

// Data
static int const NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT g_frameIndex = 0;

static int const NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12Fence* g_fence = nullptr;
static HANDLE g_fenceEvent = nullptr;
static UINT64 g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = nullptr;
static HANDLE g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS
] = {};

void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
void ResizeSwapChain(HWND hWnd, int width, int height);
#endif

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();  // D3D10, D3D11
void CleanupRenderTarget(); // D3D10, D3D11
void ResetDevice();         // D3D9 only

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Colors
constexpr float BACKCOLOR[4] = {0.94f, 0.94f, 0.94f, 1.00f};
inline HWND     hwnd;

// Main code
inline void init_graphics()
{
    // Create application window
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr),
        nullptr, nullptr, nullptr, nullptr,
        _T("Losedows"), nullptr
    };
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, _T("Losedows - Windows Toolbox"),
                          WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL,
                          NULL,
                          wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)){
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        exit(1);
    }

    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    init(io);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    BACKEND_INIT

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT){
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)){
            TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        BACKEND_NEW_FRAME();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        draw();

        // Rendering
        ImGui::EndFrame();
#if defined(BACKEND_USE_D3D9)
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(static_cast<int>(BACKCOLOR[0] * 255.0f),
      static_cast<int>(BACKCOLOR[1] * 255.0f),
      static_cast<int>(BACKCOLOR[2] * 255.0f),
      static_cast<int>(BACKCOLOR[3] * 255.0f
        ));
    g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
      clear_col_dx, 1.0f, 0);
    if (g_pd3dDevice->BeginScene() >= 0) {
      ImGui::Render();
      ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
      g_pd3dDevice->EndScene();
    }
    HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

    // Handle loss of D3D9 device
    if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() ==
      D3DERR_DEVICENOTRESET) {
      ResetDevice();
    }
#elif defined(BACKEND_USE_D3D10)
        // Render
        ImGui::Render();
        g_pd3dDevice->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDevice->ClearRenderTargetView(g_mainRenderTargetView, BACKCOLOR);
        ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
#elif defined(BACKEND_USE_D3D11)
    // Render
    ImGui::Render();
    g_pd3dDeviceContext->
      OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
      BACKCOLOR);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
#else // D3D12
    FrameContext* frameCtxt = WaitForNextFrameResources();
    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    frameCtxt->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    g_pd3dCommandList->Reset(frameCtxt->CommandAllocator, nullptr);
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->ClearRenderTargetView(
      g_mainRenderTargetDescriptor[backBufferIdx], BACKCOLOR, 0, nullptr);
    g_pd3dCommandList->OMSetRenderTargets(
      1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    g_pd3dCommandQueue->ExecuteCommandLists(
      1, (ID3D12CommandList* const*)&g_pd3dCommandList);

    g_pSwapChain->Present(1, 0); // Present with vsync

    UINT64 fenceValue = g_fenceLastSignaledValue + 1;
    g_pd3dCommandQueue->Signal(g_fence, fenceValue);
    g_fenceLastSignaledValue = fenceValue;
    frameCtxt->FenceValue = fenceValue;
#endif
    }
    // Cleanup
#ifdef BACKEND_USE_D3D12
  WaitForLastSubmittedFrame();
#endif
    BACKEND_SHUTDOWN();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

// Helper functions
inline bool CreateDeviceD3D(HWND hWnd)
{
#if defined(BACKEND_USE_D3D9)
  if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr){
    return false;
  }

  // Create the D3DDevice
  ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
  g_d3dpp.Windowed = TRUE;
  g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
  g_d3dpp.EnableAutoDepthStencil = TRUE;
  g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
  g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
  //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
  if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                           D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp,
                           &g_pd3dDevice) < 0){
    return false;
  }

  return true;
#elif defined(BACKEND_USE_D3D10) || defined(BACKEND_USE_D3D11)
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = hWnd;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
# if defined(BACKEND_USE_D3D10)
    //createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
    if (D3D10CreateDeviceAndSwapChain(nullptr, D3D10_DRIVER_TYPE_HARDWARE,
                                      nullptr, createDeviceFlags,
                                      D3D10_SDK_VERSION, &sd, &g_pSwapChain,
                                      &g_pd3dDevice) != S_OK){
        return false;
    }
# elif defined(BACKEND_USE_D3D11)
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
      D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
  if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
    createDeviceFlags, featureLevelArray,
    2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
    &g_pd3dDevice, &featureLevel,
    &g_pd3dDeviceContext) != S_OK){
    return false;
  }
# endif
#else // D3D12
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC1 sd;
  {
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = NUM_BACK_BUFFERS;
    sd.Width = 0;
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.Stereo = FALSE;
  }

  // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
  ID3D12Debug* pdx12Debug = nullptr;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
    pdx12Debug->EnableDebugLayer();
#endif

  // Create device
  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
  if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) !=
    S_OK)
    return false;

  // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
  if (pdx12Debug != nullptr) {
    ID3D12InfoQueue* pInfoQueue = nullptr;
    g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
    pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    pInfoQueue->Release();
    pdx12Debug->Release();
  }
#endif

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = NUM_BACK_BUFFERS;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 1;
    if (g_pd3dDevice->CreateDescriptorHeap(
      &desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
      return false;

    SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->
      GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
      g_mainRenderTargetDescriptor[i] = rtvHandle;
      rtvHandle.ptr += rtvDescriptorSize;
    }
  }

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (g_pd3dDevice->CreateDescriptorHeap(
      &desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
      return false;
  }

  {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 1;
    if (g_pd3dDevice->CreateCommandQueue(
      &desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
      return false;
  }

  for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
      IID_PPV_ARGS(
        &g_frameContext[i].
        CommandAllocator)) != S_OK)
      return false;

  if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
    g_frameContext[0].CommandAllocator,
    nullptr,
    IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK
    ||
    g_pd3dCommandList->Close() != S_OK)
    return false;

  if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
    IID_PPV_ARGS(&g_fence)) != S_OK)
    return false;

  g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (g_fenceEvent == nullptr)
    return false;

  {
    IDXGIFactory4* dxgiFactory = nullptr;
    IDXGISwapChain1* swapChain1 = nullptr;
    if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK ||
      dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd,
        nullptr, nullptr,
        &swapChain1) != S_OK ||
      swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
      return false;
    swapChain1->Release();
    dxgiFactory->Release();
    g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
    g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
  }
#endif
#ifndef BACKEND_USE_D3D9
    CreateRenderTarget();
#endif
    return true;
}

inline void CleanupDeviceD3D()
{
#if defined(BACKEND_USE_D3D9)
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }
  if (g_pD3D) {
    g_pD3D->Release();
    g_pD3D = nullptr;
  }
#elif defined(BACKEND_USE_D3D10)
    CleanupRenderTarget();
    if (g_pSwapChain){
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDevice){
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
#elif defined (BACKEND_USE_D3D11)
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
  }
  if (g_pd3dDeviceContext) {
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = nullptr;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }
#else // D3D12
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
  }
  if (g_hSwapChainWaitableObject != nullptr) {
    CloseHandle(g_hSwapChainWaitableObject);
  }
  for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
    if (g_frameContext[i].CommandAllocator) {
      g_frameContext[i].CommandAllocator->Release();
      g_frameContext[i].CommandAllocator = nullptr;
    }
  if (g_pd3dCommandQueue) {
    g_pd3dCommandQueue->Release();
    g_pd3dCommandQueue = nullptr;
  }
  if (g_pd3dCommandList) {
    g_pd3dCommandList->Release();
    g_pd3dCommandList = nullptr;
  }
  if (g_pd3dRtvDescHeap) {
    g_pd3dRtvDescHeap->Release();
    g_pd3dRtvDescHeap = nullptr;
  }
  if (g_pd3dSrvDescHeap) {
    g_pd3dSrvDescHeap->Release();
    g_pd3dSrvDescHeap = nullptr;
  }
  if (g_fence) {
    g_fence->Release();
    g_fence = nullptr;
  }
  if (g_fenceEvent) {
    CloseHandle(g_fenceEvent);
    g_fenceEvent = nullptr;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }

#ifdef DX12_ENABLE_DEBUG_LAYER
  IDXGIDebug1* pDebug = nullptr;
  if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug)))) {
    pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
    pDebug->Release();
  }
#endif
#endif
}

#ifdef BACKEND_USE_D3D9
inline void ResetDevice() {
  ImGui_ImplDX9_InvalidateDeviceObjects();
  HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
  if (hr == D3DERR_INVALIDCALL)
    IM_ASSERT(0);
  ImGui_ImplDX9_CreateDeviceObjects();
}
#endif

#if defined(BACKEND_USE_D3D10) || defined(BACKEND_USE_D3D11)
inline void CreateRenderTarget()
{
    TEXTURE2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                         &g_mainRenderTargetView);
    pBackBuffer->Release();
}

inline void CleanupRenderTarget()
{
    if (g_mainRenderTargetView){
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}
#endif

#ifdef BACKEND_USE_D3D12
inline void CreateRenderTarget() {
  for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
    ID3D12Resource* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
      g_mainRenderTargetDescriptor[i]);
    g_mainRenderTargetResource[i] = pBackBuffer;
  }
}

inline void CleanupRenderTarget() {
  WaitForLastSubmittedFrame();

  for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    if (g_mainRenderTargetResource[i]) {
      g_mainRenderTargetResource[i]->Release();
      g_mainRenderTargetResource[i] = nullptr;
    }
}

inline void WaitForLastSubmittedFrame() {
  FrameContext* frameCtxt = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT
  ];

  UINT64 fenceValue = frameCtxt->FenceValue;
  if (fenceValue == 0)
    return; // No fence was signaled

  frameCtxt->FenceValue = 0;
  if (g_fence->GetCompletedValue() >= fenceValue)
    return;

  g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
  WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources() {
  UINT nextFrameIndex = g_frameIndex + 1;
  g_frameIndex = nextFrameIndex;

  HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
  DWORD numWaitableObjects = 1;

  FrameContext* frameCtxt = &g_frameContext[
    nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
  UINT64 fenceValue = frameCtxt->FenceValue;
  if (fenceValue != 0) // means no fence was signaled
  {
    frameCtxt->FenceValue = 0;
    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    waitableObjects[1] = g_fenceEvent;
    numWaitableObjects = 2;
  }

  WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

  return frameCtxt;
}

inline void ResizeSwapChain(HWND hWnd, int width, int height) {
  DXGI_SWAP_CHAIN_DESC1 sd;
  g_pSwapChain->GetDesc1(&sd);
  sd.Width = width;
  sd.Height = height;

  IDXGIFactory4* dxgiFactory = nullptr;
  g_pSwapChain->GetParent(IID_PPV_ARGS(&dxgiFactory));

  g_pSwapChain->Release();
  CloseHandle(g_hSwapChainWaitableObject);

  IDXGISwapChain1* swapChain1 = nullptr;
  dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, nullptr,
    nullptr, &swapChain1);
  swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
  swapChain1->Release();
  dxgiFactory->Release();

  g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);

  g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
  assert(g_hSwapChainWaitableObject != NULL);
}
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg){
    case WM_SIZE:
#if defined(BACKEND_USE_D3D9)
    if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
      g_d3dpp.BackBufferWidth = LOWORD(lParam);
      g_d3dpp.BackBufferHeight = HIWORD(lParam);
      ResetDevice();
    }
#elif defined(BACKEND_USE_D3D10) || defined(BACKEND_USE_D3D11)
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED){
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, static_cast<UINT>(LOWORD(lParam)),
                                        static_cast<UINT>(HIWORD(lParam)),
                                        DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
#else // D3D12
    if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
      WaitForLastSubmittedFrame();
      ImGui_ImplDX12_InvalidateDeviceObjects();
      CleanupRenderTarget();
      ResizeSwapChain(hWnd, static_cast<UINT>(LOWORD(lParam)),
        static_cast<UINT>(HIWORD(lParam)));
      CreateRenderTarget();
      ImGui_ImplDX12_CreateDeviceObjects();
  }
#endif
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
