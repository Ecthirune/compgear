#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <dwmapi.h>
#include <windows.h>
#include "data/affix_sorter.h"
#include <fstream>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

extern bool imgui_init();

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
  // dpi масштабирование
  typedef BOOL(WINAPI * SetProcessDpiAwarenessContextFunc)(HANDLE);
  auto fn = reinterpret_cast<SetProcessDpiAwarenessContextFunc>(GetProcAddress(
      GetModuleHandleW(L"user32.dll"), "SetProcessDpiAwarenessContext"));
  if (fn) fn((HANDLE)-4);  // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
      // if (create_data_files("input.json")){
      // }
      // else{
      //   MessageBoxA(NULL, "Can't find input.json, terminating", "Error", MB_OK | MB_ICONERROR);
      //   return 1;
      // };
  return imgui_init() ? 0 : 1;
}
