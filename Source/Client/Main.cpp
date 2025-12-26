#include "Client/Game/UFunction.hpp"
#include "Client/Game/UWorld.hpp"
#include "Client/Hooks/SymbolHook.hpp"
#include "Common/String.hpp"
#include <ios>

uintptr_t Game::ImageBase = 0;

SymbolHook<{ "?InitWorld@UWorld@@QEAAXUInitializationValues@1@@Z" }> InitWorldHook;
SymbolHook<{ "?Invoke@UFunction@@QEAAXPEAVUObject@@AEAUFFrame@@QEAX@Z" }> FuncInvokeHook;

static void PromoteDebugger() {
  MessageBoxW(nullhandle, L"Attach your debugger", L"", MB_OK);
}

static void InitConsole() {
  AllocConsole();

  DWORD consoleMode;
  HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleMode(output, &consoleMode);
  SetConsoleMode(output, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  SetConsoleOutputCP(CP_UTF8);

  freopen("CONOUT$", "w+", stdout);
  freopen("CONOUT$", "w+", stderr);
  std::ios::sync_with_stdio(true);
}

static void Initialize() {
  InitConsole();
  Logger::Initialize("VotV-MP.log");
  Log::Info("Starting things up");

  Game::ImageBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
  Log::Info("Game image base is {}", reinterpret_cast<void*>(Game::ImageBase));

  InitWorldHook.Callback = [](UWorld* _this, UWorld::InitializationValues initValues) {
    InitWorldHook.GetTrampoline()(_this, initValues);

    auto mapName = _this->GetMapName();
    Log::Debug("New world {} initialized with map {}", fmt::ptr(_this), String(mapName.buffer));
  };
  InitWorldHook.Install();

  FuncInvokeHook.Install();
}

BOOL WINAPI DllMain(HINSTANCE library, DWORD reason, BOOL isSysCall) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(library);
    PromoteDebugger();
    Initialize();
  }
  else if (reason == DLL_PROCESS_DETACH) {
    if (!isSysCall) {
      MessageBoxW(nullhandle, L"The fuck are you doing?", L"", 0);
      TerminateProcess(GetCurrentProcess(), 0);
    }
  }

  return TRUE;
}
