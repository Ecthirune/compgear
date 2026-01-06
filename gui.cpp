#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <dwmapi.h>
#include <windows.h>

#include <string>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

// imgui inc
#include "controller.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/imgui.h"

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static HWND g_hwnd = nullptr;
static bool g_showGui = false;

#define HOTKEY_ID_F11 1001

void CleanupRenderTarget();
void CleanupDeviceD3D();
bool CreateDeviceD3D(HWND hWnd);
void CreateRenderTarget();

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (g_showGui && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
    case WM_HOTKEY:
      if (wParam == HOTKEY_ID_F11) {
        g_showGui = !g_showGui;
        if (g_showGui) {
          ShowWindow(g_hwnd, SW_SHOW);
          SetForegroundWindow(g_hwnd);
          SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        } else {
          ShowWindow(g_hwnd, SW_HIDE);
        }
        OutputDebugStringW(g_showGui ? L"[GUI] Shown\n" : L"[GUI] Hidden\n");
        return 0;
      }
      break;
    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT && g_showGui) {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return TRUE;
      }
      break;
    case WM_DESTROY:
      UnregisterHotKey(hWnd, HOTKEY_ID_F11);
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CleanupRenderTarget() {
  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
  }
}

void CleanupDeviceD3D() {
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
}

void CreateRenderTarget() {
  ID3D11Texture2D* pBackBuffer = nullptr;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                       &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void RenderUI() {
   
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoCollapse | 
                             ImGuiWindowFlags_NoBackground | 
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("OverlayWindow", nullptr, flags);

 
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "CompGear"); 
    ImGui::Separator();

    if (ImGui::Button("Insert data", ImVec2(-1, 40))) {
        
    }
    
   
    ImGui::End();
}

bool CreateDeviceD3D(HWND hWnd) {
  DXGI_SWAP_CHAIN_DESC sd = {};
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  D3D_FEATURE_LEVEL featureLevel = {};
  const D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };

  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 2,
      D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel,
      &g_pd3dDeviceContext);

  if (FAILED(res)) {
    return false;
  }

  CreateRenderTarget();
  return true;
}
bool imgui_init() {
  WNDCLASSEX wc = {sizeof(wc)};
  wc.style = CS_CLASSDC;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = L"CompGearClass";
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassEx(&wc);

  g_hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, wc.lpszClassName,
                          L"CompGear", WS_POPUP, 100, 100, 400, 100, nullptr,
                          nullptr, wc.hInstance, nullptr);
  SetLayeredWindowAttributes(g_hwnd, 0, 255, LWA_ALPHA);
  MARGINS margins = {-1};
  DwmExtendFrameIntoClientArea(g_hwnd, &margins);
  if (!g_hwnd) {
    OutputDebugStringW(L"[ERROR] CreateWindowEx failed!\n");
    return false;
  }

  if (!RegisterHotKey(g_hwnd, HOTKEY_ID_F11, 0, VK_F11)) {
    OutputDebugStringW(L"[WARN] RegisterHotKey failed!\n");
  }

  ShowWindow(g_hwnd, SW_HIDE);

  if (!CreateDeviceD3D(g_hwnd)) {
    OutputDebugStringW(L"[ERROR] CreateDeviceD3D failed!\n");
    CleanupDeviceD3D();
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(g_hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  
  MSG msg = {};
  bool done = false;

  while (!done)
{
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) done = true;
    }
    if (done) break;

    if (g_showGui)
    {
        
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderUI(); 

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); 
        
       
        if (ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        Sleep(5); 
    } else {
        Sleep(16); // ~60 FPS
    }
    }
    else
    {
        WaitMessage(); 
    }
}

 
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDeviceD3D();
  UnregisterHotKey(g_hwnd, HOTKEY_ID_F11);
  DestroyWindow(g_hwnd);
  UnregisterClass(wc.lpszClassName, wc.hInstance);

  return true;
}