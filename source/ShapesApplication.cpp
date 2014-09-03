#include "ShapesApplication.h"
#include "SpheresLayer.h"
#include "SpaceLayer.h"
#include "GridLayer.h"
#include "SDL.h"

#include <cassert>
#include <thread>

#include "gl_glext_glu.h"

#if _WIN32
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ws2_32.lib")
#endif

#include "Leap.h"

// There is a really dumb compile error on Linux: "Eigen/src/Core/util/Constants.h:369:2:
// error: #error The preprocessor symbol 'Success' is defined, possibly by the X11 header file X.h",
// so this undef is necessary until we can figure out a better solution.
#if defined(Success)
#undef Success
#endif

void ShapesApplication::Initialize() {
  m_Selected = 0;

  SDLControllerParams params;
  params.transparentWindow = false;
  params.fullscreen = false;
  params.windowTitle = "Internal Visualizer";

  m_applicationTime = TimePoint(0.0);         // Start the application time at zero.
  m_SDLController.Initialize(params);         // This initializes everything SDL-related.
  m_GLController.Initialize();                // This initializes the general GL state.
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Glew initialization failed");
  }

#if _WIN32
  m_Oculus.SetHWND(m_SDLController.GetHWND());
#endif
  if (!m_Oculus.Init()) {
    throw std::runtime_error("Oculus initialization failed");
  }
  m_Oculus.DismissHealthWarning();
  m_LeapController.addListener(m_LeapListener);
  m_LeapController.setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES);
  m_LeapController.setPolicyFlags(Leap::Controller::POLICY_IMAGES);

  InitializeApplicationLayers();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ShapesApplication::Shutdown() {
  m_LeapController.removeListener(m_LeapListener);

  ShutdownApplicationLayers();                // Destroy the application layers, from top (last) to bottom (first).
  m_GLController.Shutdown();                  // This shuts down the general GL state.
  m_SDLController.Shutdown();                // This shuts down everything SDL-related.
}

void ShapesApplication::Update(TimeDelta real_time_delta) {
  assert(real_time_delta >= 0.0);
  m_applicationTime += real_time_delta;         // Increment the application time by the delta.

  //m_LeapListener.WaitForFrame();
  const Leap::Frame& frame = m_LeapController.frame();

  float leap_baseline = 40.0f;
  // Set passthrough images
  const Leap::ImageList& images = frame.images();
  if (images.count() == 2) {
    for (int i = 0; i < 2; i++) {
      if (images[i].width() == 640) {
        m_PassthroughLayer[i]->SetImage(images[i].data());
      } else {
        m_PassthroughLayer[i]->SetColorImage(images[i].data());
        leap_baseline = 64.0f;
      }
      m_PassthroughLayer[i]->SetDistortion(images[i].distortion());
    }
  }

  // Calculate where each point of interest would have to be if a 6.4-cm baseline Leap centered exactly at the eyeballs saw the frame seen. It will be off by a factor of 1.6.
  // This is done using the following formula:
  const float OCULUS_BASELINE = 0.064f; // TODO: Get this value directly from the SDK

  //TODO: This can be obtained by calling ovrHmd_GetSensorState with ScanoutMidpointSeconds for absolute time.
  const Matrix4x4f avgView = 0.5f*(m_Oculus.EyeView(0) + m_Oculus.EyeView(1));
  Matrix4x4f inputTransform = avgView.inverse();
  Matrix3x3f conventionConv;
  conventionConv << -Vector3f::UnitX(), -Vector3f::UnitZ(), -Vector3f::UnitY();
  inputTransform.block<3, 3>(0, 0) *= OCULUS_BASELINE/leap_baseline*conventionConv;

  for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it) {
    InteractionLayer &layer = **it;
    if (layer.Alpha() > 0.01f) {
      layer.UpdateLeap(frame, inputTransform);
      layer.UpdateEyePos(avgView.inverse().block<3, 1>(0, 3));
      layer.Update(real_time_delta);              // Update each application layer, from back to front.
    }
  }
}

void ShapesApplication::Render(TimeDelta real_time_delta) const {
  assert(real_time_delta >= 0.0);
  m_Oculus.BeginFrameAPI();
  m_Oculus.BeginFrame();

  //glViewport(1920, 1080-432, 384, 432);
  //RenderEye(real_time_delta, 0);
  //return;

  //std::cout << __LINE__ << " -> " << (0.06*SDL_GetTicks()) << "\n";
  //std::cout << __LINE__ << "    -> " << (0.06*SDL_GetTicks()) << std::endl;
  //m_GLController.BeginRender();               // NOTE: ALL RENDERING should go between here and EndRender().

  // Do the eye-order trick!
  for (int i = 0; i < 2; i++) {
    const ovrRecti& rect = m_Oculus.EyeViewport(i);
    glViewport(rect.Pos.x, rect.Pos.y, rect.Size.w, rect.Size.h);
    RenderEye(real_time_delta, i);
  }

  //m_GLController.EndRender();                 // NOTE: ALL RENDERING should go between here and BeginRender().
  //std::cout << __LINE__ << "       -> " << (0.06*SDL_GetTicks()) << std::endl;
  m_Oculus.EndFrame();
  //std::cout << __LINE__ << "          -> " << (0.06*SDL_GetTicks()) << std::endl;
  return;
}

void ShapesApplication::RenderEye(TimeDelta real_time_delta, int i) const {
  const Matrix4x4f proj = m_Oculus.EyeProjection(i);
  const Matrix4x4f view = m_Oculus.EyeView(i);

  // Set default shader's state
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj.data());
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view.data());

  m_PassthroughLayer[i]->GetRenderState().GetProjection().Matrix() = proj.cast<double>();
  m_PassthroughLayer[i]->Render(real_time_delta);

  glEnable(GL_DEPTH_TEST);
  for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it) {
    // Set individual shader's state
    InteractionLayer &layer = **it;
    if (layer.Alpha() > 0.01f) {
      layer.GetRenderState().GetProjection().Matrix() = proj.cast<double>();
      layer.GetRenderState().GetModelView().Matrix() = view.cast<double>();
      layer.Render(real_time_delta);              // Render each application layer, from back to front.
    }
  }
}

EventHandlerAction ShapesApplication::HandleWindowEvent(const SDL_WindowEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_WindowEvent>(ev, &EventHandler::HandleWindowEvent);
}

EventHandlerAction ShapesApplication::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  if (ev.type == SDL_KEYDOWN) {
    switch (ev.keysym.sym) {
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
    case SDLK_7:
      m_Selected = (int)(ev.keysym.sym - SDLK_1) % m_Layers.size();
      break;
    case SDLK_UP: {
      float& alpha = m_Layers[m_Selected]->Alpha();
      alpha = std::min(1.0f, alpha + 0.025f);
    }
    break;
    case SDLK_DOWN: {
      float& alpha = m_Layers[m_Selected]->Alpha();
      alpha = std::max(0.0f, alpha - 0.025f);
    }
    break;
    case SDLK_F11:
      m_SDLController.ToggleFullscreen();
      break;
    case 0x1b:
      exit(0);
    default:
      break;
    }
  }
  return DispatchEventToApplicationLayers<SDL_KeyboardEvent>(ev, &EventHandler::HandleKeyboardEvent);
}

EventHandlerAction ShapesApplication::HandleMouseMotionEvent(const SDL_MouseMotionEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_MouseMotionEvent>(ev, &EventHandler::HandleMouseMotionEvent);
}

EventHandlerAction ShapesApplication::HandleMouseButtonEvent(const SDL_MouseButtonEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_MouseButtonEvent>(ev, &EventHandler::HandleMouseButtonEvent);
}

EventHandlerAction ShapesApplication::HandleMouseWheelEvent(const SDL_MouseWheelEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_MouseWheelEvent>(ev, &EventHandler::HandleMouseWheelEvent);
}

EventHandlerAction ShapesApplication::HandleQuitEvent(const SDL_QuitEvent &ev) {
  exit(0);
}

EventHandlerAction ShapesApplication::HandleGenericSDLEvent(const SDL_Event &ev) {
  return DispatchEventToApplicationLayers<SDL_Event>(ev, &EventHandler::HandleGenericSDLEvent);
}

TimePoint ShapesApplication::Time() const {
  return m_applicationTime;
}

void ShapesApplication::InitializeApplicationLayers() {
  m_Layers.push_back(std::shared_ptr<GridLayer>(new GridLayer));
  m_Layers.push_back(std::shared_ptr<SpheresLayer>(new SpheresLayer));
  m_Layers.push_back(std::shared_ptr<SpaceLayer>(new SpaceLayer));
  //m_Layers.push_back(std::shared_ptr<QuadsLayer>(new QuadsLayer));
  for (int i = 0; i < 2; i++) {
    m_PassthroughLayer[i] = std::shared_ptr<PassthroughLayer>(new PassthroughLayer());
  }
}

void ShapesApplication::ShutdownApplicationLayers() {
  // Destroy the application-specific layers, in reverse order.
  for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
    it->reset();  // destroy the layer by resetting its shared_ptr.
  }
  m_Layers.clear();

  for (int i = 0; i < 2; i++) {
    m_PassthroughLayer[i].reset();
  }
}
