#pragma once

#include "Client/Game/FString.hpp"
#include "Client/Game/Game.hpp"
#include <Windows.h>

// clang-format off
DEFINE_SYMBOL("WinMain", 0x7F3E30, int(__stdcall*)(HINSTANCE, HINSTANCE, LPSTR, int));
DEFINE_SYMBOL("?appGetStartupMap@@YA?AVFString@@PEB_W@Z", 0x2F29A40, FString(*)(const wchar_t* commandLine));
DEFINE_SYMBOL("?GetMapNameStatic@@YA?BVFString@@XZ", 0x2F6B0D0, FString(*)());
// clang-format on

namespace Global {

forceinline int WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  int nShowCmd
) {
  return Game::CallSymbol<{ "WinMain" }>(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

forceinline FString appGetStartupMap(const wchar_t* commandLine) {
  return Game::CallSymbol<{ "?appGetStartupMap@@YA?AVFString@@PEB_W@Z" }>(commandLine);
}

forceinline FString GetMapNameStatic() {
  return Game::CallSymbol<{ "?GetMapNameStatic@@YA?BVFString@@XZ" }>();
}

} // namespace Global
