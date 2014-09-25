#include "stdafx.h"
#include "VRIntroApp.h"

Uint32 SDL_Window_ID;

int main(int argc, char **argv) {
  bool showMirror = argc >= 2 && strcmp(argv[1], "mirror") == 0;

  VRIntroApp app(showMirror);
  RunApplication(app);
  return 0;
}
