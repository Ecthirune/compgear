
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <dwmapi.h>
#include <windows.h>

#include <algorithm>
#include <string>
#include <sstream>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")


#include "../controller/controller.h"
#include "../include/imgui/backends/imgui_impl_dx11.h"
#include "../include/imgui/backends/imgui_impl_win32.h"
#include "../include/imgui/imgui.h"

#define HOTKEY_ID_F11 1001
#pragma execution_character_set("utf-8")

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static HWND g_hwnd = nullptr;
static bool g_showGui = false;

// Добавляем переменные для управления размером окна
static bool g_windowSizeNeedsUpdate = false;
static ImVec2 g_windowMinSize = ImVec2(1000, 1000);
static ImVec2 g_windowTargetSize = ImVec2(1000, 1000);

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
    case WM_SIZE:
      if (wParam != SIZE_MINIMIZED) {
        CleanupRenderTarget();
        if (g_pSwapChain) {
          g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam),
                                      DXGI_FORMAT_UNKNOWN, 0);
          CreateRenderTarget();
          ImGui_ImplDX11_InvalidateDeviceObjects();
          ImGui_ImplDX11_CreateDeviceObjects();
        }
      }
      return 0;
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

enum class GuiState { MainMenu, Editor };
static GuiState g_currentState = GuiState::MainMenu;
  static int selected_affix_idx = 0;

bool PassFilter(const std::string& affix, const std::string& filter) {
    if (filter.empty()) return true;
    
    std::stringstream ss(filter);
    std::string token;
    while (ss >> token) {
        if (token.length() < 3) continue;
        
       
        auto it = std::search(
            affix.begin(), affix.end(),
            token.begin(), token.end(),
            [](char a, char b) { return std::tolower(a) == std::tolower(b); }
        );
        
        if (it == affix.end()) return false; 
    }
    return true;
}

void RenderEditor()
{
  if (ImGui::Button("<< Back to Menu")) g_currentState = GuiState::MainMenu;

    std::vector<std::string> bases = Controller::GetItemBases();
  if (bases.empty()) return;

  static int selected_idx = 0;
  static bool init = false;
  if (!init) {
    Controller::SetItemBase(bases[selected_idx]);
    init = true;
  }

  ImGui::Begin("Affix Selector");
  bool changed = false;
  if (ImGui::BeginCombo("Item Base", bases[selected_idx].c_str())) {
    for (int n = 0; n < bases.size(); n++) {
      if (ImGui::Selectable(bases[n].c_str(), selected_idx == n)) {
        selected_idx = n;
        Controller::SetItemBase(bases[n]);
        changed = true;
        
      }
    }
    ImGui::EndCombo();
  }
  static auto available_affixes = Controller::RefreshAffixes();
  if (Controller::NeedsAttributeSelector(bases[selected_idx])) {
    ImGui::Separator();
    ImGui::Text("Base Attributes:");
    static bool s_str = false, s_dex = false, s_int = false;

    

    if (ImGui::Checkbox("STR", &s_str)) {
      Controller::ToggleStat("str", s_str);
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("DEX", &s_dex)) {
      Controller::ToggleStat("dex", s_dex);
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("INT", &s_int)) {
      Controller::ToggleStat("int", s_int);
      changed = true;
    }

   
  }
 if (changed) {
      selected_affix_idx = 0;
      available_affixes = Controller::RefreshAffixes();
    }

  if (!available_affixes.empty()) {
   static char filter_buffer[128] = "";
ImGui::InputText("Search Affix (min 3 chars)", filter_buffer, IM_ARRAYSIZE(filter_buffer));

static int selected_affix_idx = 0;
std::vector<int> filtered_indices;

// Фильтруем индексы доступных аффиксов
for (int i = 0; i < available_affixes.size(); ++i) {
    if (PassFilter(available_affixes[i], filter_buffer)) {
        filtered_indices.push_back(i);
    }
}

if (!filtered_indices.empty()) {
    if (ImGui::BeginListBox("##AffixList", ImVec2(-FLT_MIN, 150))) {
        for (int i = 0; i < filtered_indices.size(); ++i) {
            int original_idx = filtered_indices[i];
            bool is_selected = (selected_affix_idx == original_idx);
            
            if (ImGui::Selectable(available_affixes[original_idx].c_str(), is_selected)) {
                selected_affix_idx = original_idx;
            }
            
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }

    if (ImGui::Button("Add Affix to Preset")) {
        if (selected_affix_idx < available_affixes.size()) {
            Controller::AddSelectedAffixToPreset(available_affixes[selected_affix_idx]);
        }
    }
} else {
    ImGui::TextDisabled("No affixes match your search.");
}
  }
  if (ImGui::Button("Add Affix to Preset")) {
    if (!available_affixes.empty()) {
      Controller::AddSelectedAffixToPreset(
          available_affixes[selected_affix_idx]);
    }
  }

  ImGui::Separator();

  ImGui::TextColored(ImVec4(1, 1, 0, 1), "YOUR PRESET:");

  const auto& preset_content = Controller::GetCurrentPresetContent();

  for (const auto& item : preset_content) {
    if (ImGui::TreeNode(item.tag_name.c_str())) {
      for (const auto& aff : item.affixes) {
        ImGui::BulletText("%s", aff.c_str());
      }
      ImGui::TreePop();
    }
  }

  ImGui::End();
}

void RenderMainMenu() {
    ImGui::Begin("PoE2 Preset Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    if (ImGui::Button("A) Create New Preset", ImVec2(300, 40))) {
        Controller::NewPreset();
        g_currentState = GuiState::Editor;
    }

    ImGui::Separator();
    ImGui::Text("B) Edit Existing Preset:");
    
    auto presets = Controller::GetPresetList();
    if (presets.empty()) {
        ImGui::TextDisabled("No presets found in folder...");
    } else {
        for (const auto& name : presets) {
            if (ImGui::Button(name.c_str(), ImVec2(300, 0))) {
                Controller::LoadPreset(name);
                g_currentState = GuiState::Editor;
            }
        }
    }

    ImGui::End();
}

void RenderUI() {
    if (g_currentState == GuiState::MainMenu) {
        RenderMainMenu();
    } else {
        RenderEditor();
    }
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

  // Изменяем стиль окна, чтобы сделать его изменяемым
  DWORD windowStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;

  g_hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, wc.lpszClassName,
                          L"CompGear", windowStyle, 100, 100, 1000, 1000,
                          nullptr, nullptr, wc.hInstance, nullptr);

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

  while (!done) {
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) done = true;
    }
    if (done) break;

    if (g_showGui) {
      // Обновляем размер окна, если нужно
      if (g_windowSizeNeedsUpdate) {
        RECT windowRect;
        GetWindowRect(g_hwnd, &windowRect);
        int x = windowRect.left;
        int y = windowRect.top;

        SetWindowPos(g_hwnd, HWND_TOPMOST, x, y, (int)g_windowTargetSize.x,
                     (int)g_windowTargetSize.y, SWP_NOZORDER | SWP_SHOWWINDOW);

        g_windowSizeNeedsUpdate = false;
      }

      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();

      RenderUI();

      ImGui::Render();
      const float clear_color_with_alpha[4] = {0.f, 0.f, 0.f, 0.f};
      g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView,
                                              nullptr);
      g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                                 clear_color_with_alpha);
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

      g_pSwapChain->Present(1, 0);

      if (ImGui::IsAnyItemActive() ||
          ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        Sleep(5);
      } else {
        Sleep(16);  // ~60 FPS
      }
    } else {
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
