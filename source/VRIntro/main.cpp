#include "stdafx.h"
#include "VRIntroApp.h"
#include "APIFrameSupplier.h"

Uint32 SDL_Window_ID;

int main(int argc, char **argv) {
  bool showMirror = argc >= 2 && strcmp(argv[1], "mirror") == 0;
  
  APIFrameSupplier supplier;

  VRIntroApp app(showMirror);
  app.SetFrameSupplier(&supplier);
  RunApplication(app);
  return 0;
}
