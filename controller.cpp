#include "controller.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

std::wstring Controller::GetClipboardText() {
  if (!OpenClipboard(nullptr)) return L"";

  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  if (!hData) {
    CloseClipboard();
    return L"";
  }
  const wchar_t* psz = static_cast<const wchar_t*>(GlobalLock(hData));
  std::wstring data = psz ? psz : L"";
  GlobalUnlock(hData);
  CloseClipboard();
  return data;
}

void Controller::ProcessClipboard() {
  std::wstring text = GetClipboardText();
  if (text.empty()) {
    OutputDebugString(L"[Controller] Clipboard is empty\n");
    return;
  }
}