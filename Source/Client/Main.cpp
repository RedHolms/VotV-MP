static void Initialize() {

}

BOOL WINAPI DllMain(HINSTANCE library, DWORD reason, BOOL isSysCall) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(library);
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
