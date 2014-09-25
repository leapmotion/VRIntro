#include "stdafx.h"
#include "APIFrameSupplier.h"
#include "VRIntroLib/VRIntroApp.h"

Uint32 SDL_Window_ID;

int main(int argc, char **argv) {
  bool showMirror = argc >= 2 && strcmp(argv[1], "mirror") == 0;
  
  APIFrameSupplier supplier;

  VRIntroApp app(showMirror);
  app.SetFrameSupplier(&supplier);
  app.Run();

  return 0;
}
