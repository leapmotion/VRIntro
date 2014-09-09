//#include "stdafx.h"
#include "PlatformInitializerMac.h"
#include <Foundation/NSObjCRuntime.h>
#include <mach-o/dyld.h>
#include <objc/runtime.h>
#include <unistd.h>
#include <iostream>

PlatformInitializer::PlatformInitializer(void)
{
  // Change the current directory to be that of the either the executable or,
  // preferably, the Resources directory if the executable is within an
  // application bundle.
  char exec_path[PATH_MAX] = {0};
  uint32_t pathSize = sizeof(exec_path);
  if (!_NSGetExecutablePath(exec_path, &pathSize)) {
    char fullpath[PATH_MAX] = {0};
    if (realpath(exec_path, fullpath)) {
      std::string path(fullpath);
      size_t pos = path.find_last_of('/');

      if (pos != std::string::npos) {
        path.erase(pos+1);
      }
      if (!path.empty()) {
        chdir(path.c_str());
      }
      const char* resources = "../Resources";
      if (!access(resources, R_OK)) {
        chdir(resources);
      }
    }
  }

  //
  // The isOpaque method in the SFOpenGLView class of SFML always returns YES
  // (as it just uses the default implementation of NSOpenGLView). This
  // causes us to always get an opaque view. We workaround this problem by
  // replacing that method with our own implementation that returns the
  // opaqueness based on the enclosing window, all thanks to the power of
  // Objective-C.
  //
  method_setImplementation(class_getInstanceMethod(NSClassFromString(@"SFOpenGLView"), @selector(isOpaque)),
                           imp_implementationWithBlock(^BOOL(id self, id arg) { return NO; }));
}

PlatformInitializer::~PlatformInitializer(void)
{

}

