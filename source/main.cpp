#include "VRIntroApp.h"

#if _WIN32
#include "Mirror.h"
#endif

Uint32 SDL_Window_ID;

void DispatchEventToApplication(const SDL_Event &ev, Application &app) {
  // https://wiki.libsdl.org/SDL_Event?highlight=%28%5CbCategoryStruct%5Cb%29%7C%28SDLStructTemplate%29
  switch (ev.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      app.HandleKeyboardEvent(ev.key);
      break;

    case SDL_MOUSEMOTION:
      app.HandleMouseMotionEvent(ev.motion);
      break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      app.HandleMouseButtonEvent(ev.button);
      break;

    case SDL_MOUSEWHEEL:
      app.HandleMouseWheelEvent(ev.wheel);
      break;

    case SDL_QUIT:
      app.HandleQuitEvent(ev.quit);
      break;

    case SDL_WINDOWEVENT:
      if (ev.window.windowID == SDL_Window_ID)
        app.HandleWindowEvent(ev.window);
      break;
    default:
      app.HandleGenericSDLEvent(ev);
      break;
  }
}

void RunApplication(Application &app) {
  // Give the application a chance to initialize itself.
  
  // Run the game loop until a "quit" has been requested.
  TimePoint previousRealTime(0.001 * SDL_GetTicks());
  do {
    // Handle all queue'd events.
    {
      SDL_Event ev;
      while (SDL_PollEvent(&ev)) {
        DispatchEventToApplication(ev, app);
      }
    }
    // TODO: compute the realtime using std::chrono::time_point and time deltas using std::chrono::duration
    TimePoint currentRealTime(0.001 * SDL_GetTicks());
    TimeDelta real_time_delta(currentRealTime - previousRealTime);

    // Update the application.
    app.Update(real_time_delta);
    // Render the application.
    app.Render(real_time_delta);

    // Save off the updated time for the next loop iteration.
    previousRealTime = currentRealTime;
  } while (!SDL_QuitRequested());

}

int main(int argc, char **argv) {
  VRIntroApp app;
  // VRIntroApp::Initialize is what sets everything up,
  // and VRIntroApp::Shutdown is what tears it down.
  // This call to RunApplication is what drives the application (it
  // contains e.g. the game loop, with event handling, etc).

  std::thread thread;

  app.Initialize();
#if _WIN32
  if (argc >= 2 && strcmp(argv[1], "mirror") == 0) {
    thread = std::thread(RunMirror, app.GetHwnd());
  }
#endif

  SDL_Window_ID = app.GetWindowID();
  
  RunApplication(app);

  if (thread.joinable()) {
    thread.join();
  }
  app.Shutdown();
  return 0;
}
