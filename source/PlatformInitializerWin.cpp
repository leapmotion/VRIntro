#include "stdafx.h"
#include "PlatformInitializerWin.h"
#include "shlwapi.h"
#include <Objbase.h>
#include <stdexcept>

PlatformInitializer::PlatformInitializer(void) :
  m_hr(CoInitializeEx(nullptr, COINIT_MULTITHREADED))
{
  if(FAILED(m_hr))
    throw std::runtime_error("Failed to initialize COM for multithreading");

  const size_t MAX_UTF8_BYTES = MAX_PATH * 2;
  WCHAR path[MAX_UTF8_BYTES];

  //Compute the exe path.
  const DWORD len = GetModuleFileNameW(nullptr, path, MAX_UTF8_BYTES);

  if (len == 0) {
    throw std::runtime_error("Couldn't locate our .exe");
  }

  PathRemoveFileSpecW(path);
  SetCurrentDirectoryW(path);
}

PlatformInitializer::~PlatformInitializer(void)
{
  if(SUCCEEDED(m_hr))
    CoUninitialize();
}