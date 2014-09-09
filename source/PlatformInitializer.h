#pragma once
#if _WIN32
#include "PlatformInitializerWin.h"
#elif __APPLE__
#include "PlatformInitializerMac.h"
#endif
