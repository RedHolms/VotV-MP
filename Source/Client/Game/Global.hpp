#pragma once

#include <Windows.h>
#include "base.hpp"
#include "FString.hpp"

namespace Global {

  constexpr inline Game::Function<int(__stdcall*)(HINSTANCE, HINSTANCE, LPSTR, int)> WinMain {
    "WinMain", 0x7F3E30
  };

  constexpr inline Game::Function<FString(*)(const wchar_t* commandLine)> appGetStartupMap {
    "?appGetStartupMap@@YA?AVFString@@PEB_W@Z", 0x2F29A40
  };

  constexpr inline Game::Function<FString(*)()> GetMapNameStatic {
    "?GetMapNameStatic@@YA?BVFString@@XZ", 0x2F6B0D0
  };

} // namespace Game
