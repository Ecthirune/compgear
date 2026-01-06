#pragma once
#include <string>

class Controller
{
    public:
    static std::wstring GetClipboardText();

    static void ProcessClipboard();

};