#include "stdafx.h"
#include "APIFrameSupplier.h"
#include "VRIntroLib/VRIntroApp.h"

Uint32 SDL_Window_ID;

int main(int argc, char **argv) {
  bool showMirror = (argc >= 2 && strcmp(argv[1], "mirror") == 0) || (argc >= 3 && strcmp(argv[2], "mirror") == 0);
  std::string serverUrl = argc >= 2 ? argv[1] : "ws://localhost:5000";

  APIFrameSupplier supplier;

  VRIntroApp app(serverUrl, showMirror);
  app.SetFrameSupplier(&supplier);
  app.Run();

  return 0;
}
