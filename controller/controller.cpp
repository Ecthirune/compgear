#include "controller.h"
#include <windows.h>
#include "../model/model.h"

#define WIN32_LEAN_AND_MEAN


std::string Controller::GetClipboardText() {
  if (!OpenClipboard(nullptr)) return "";

  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  if (!hData) {
    CloseClipboard();
    return "";
  }
  const wchar_t* wstr = static_cast<const wchar_t*>(GlobalLock(hData));
  if (!wstr) {
        CloseClipboard();
        return {};
    }
int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string data;
    if (len > 0) {
        data.resize(len - 1); 
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &data[0], len, nullptr, nullptr);
    }
  GlobalUnlock(hData);
  CloseClipboard();
  return data;
}

void Controller::ProcessClipboard() {
  std::string text = GetClipboardText();
  if (text.empty()) {
    OutputDebugString(L"[Controller] Clipboard is empty\n");
    return;
  }
  
}