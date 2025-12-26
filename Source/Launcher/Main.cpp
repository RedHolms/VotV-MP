#include <Shlwapi.h>
#include <Windows.h>
#include <stdio.h>

static bool SetDebugPrivilege() {
  HANDLE token;
  BOOL success;
  TOKEN_PRIVILEGES tp = { 0 }, prevTp = { 0 };
  DWORD retSize;

  success = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
  if (!success)
    return false;

  success = LookupPrivilegeValueA(nullptr, "SeDebugPrivilege", &tp.Privileges[0].Luid);
  if (!success)
    return false;

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  success = AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &prevTp, &retSize);
  if (!success)
    return false;

  CloseHandle(token);

  return true;
}

static bool LocateGameBinary(wchar_t* outPath) {
  wchar_t currentBinPath[MAX_PATH];
  GetModuleFileNameW(GetModuleHandleW(nullptr), currentBinPath, _countof(currentBinPath));

  PathRemoveFileSpecW(currentBinPath);
  PathCombineW(outPath, currentBinPath, L"VotV\\Binaries\\Win64\\VotV-Win64-Shipping.exe");

  if (PathFileExistsW(outPath))
    return true;

  return false;
}

static bool LocateMPLibrary(wchar_t* outPath) {
  wchar_t currentBinPath[MAX_PATH];
  GetModuleFileNameW(GetModuleHandleW(nullptr), currentBinPath, _countof(currentBinPath));

  PathRemoveFileSpecW(currentBinPath);
  PathCombineW(outPath, currentBinPath, L"VotV-MP-Client.dll");

  if (PathFileExistsW(outPath))
    return true;

  return false;
}

static void DoInject(HANDLE process, const wchar_t* dll) {
  void* loc =
    VirtualAllocEx(process, nullptr, MAX_PATH * 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  size_t written = 0;
  WriteProcessMemory(process, loc, dll, (wcslen(dll) + 1) * 2, &written);
  HANDLE thread =
    CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, loc, 0, nullptr);
  WaitForSingleObject(thread, INFINITE);
  CloseHandle(thread);
}

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) {
  wchar_t gamePath[MAX_PATH];
  if (!LocateGameBinary(gamePath)) {
    MessageBoxW(
      nullhandle,
      L"Не удалось найти исполняемый файл игры",
      L"VotV-MP | Ошибка запуска",
      MB_ICONERROR
    );
    return 1;
  }

  wchar_t dllPath[MAX_PATH];
  if (!LocateMPLibrary(dllPath)) {
    MessageBoxW(
      nullhandle, L"Не удалось найти DLL онлайн игры", L"VotV-MP | Ошибка запуска", MB_ICONERROR
    );
    return 1;
  }

  if (!SetDebugPrivilege()) {
    MessageBoxW(
      nullhandle,
      L"Не удалось получить debug права для процесса",
      L"VotV-MP | Ошибка запуска",
      MB_ICONERROR
    );
    return 1;
  }

  STARTUPINFOW startupInfo = {};
  startupInfo.cb = sizeof(startupInfo);
  PROCESS_INFORMATION processInfo = {};

  wchar_t cmdLine[MAX_PATH];
  swprintf(cmdLine, sizeof(cmdLine), L"\"%s\" VotV", gamePath);

  BOOL success = CreateProcessW(
    nullptr,
    cmdLine,
    nullptr,
    nullptr,
    FALSE,
    NORMAL_PRIORITY_CLASS | DETACHED_PROCESS | CREATE_SUSPENDED,
    nullptr,
    nullptr,
    &startupInfo,
    &processInfo
  );

  if (!success) {
    MessageBoxW(nullptr, L"Не удалось запустить игру", L"VotV-MP | Ошибка запуска", MB_ICONERROR);
    return 1;
  }

  DoInject(processInfo.hProcess, dllPath);

  // Start this shit up!
  ResumeThread(processInfo.hThread);

  CloseHandle(processInfo.hThread);
  CloseHandle(processInfo.hProcess);

  return 0;
}
