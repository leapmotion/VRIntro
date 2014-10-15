#include "stdafx.h"
#include "IFrameSupplier.h"
#include "VRIntroApp.h"
#include "SpheresLayer.h"
#include "SpaceLayer.h"
#include "HandLayer.h"
#include "GridLayer.h"
#include "MessageLayer.h"
#include "FractalLayer.h"
#include "QuadsLayer.h"
#include "FlyingLayer.h"
#include "SDL.h"
#include "PlatformInitializer.h"
#include "PrecisionTimer.h"

#define FREEIMAGE_LIB
#include "FreeImage.h"

#include <cassert>
#include <thread>

#include "gl_glext_glu.h"

#if _WIN32
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ws2_32.lib")

#include "Mirror.h"
#endif

// There is a really dumb compile error on Linux: "Eigen/src/Core/util/Constants.h:369:2:
// error: #error The preprocessor symbol 'Success' is defined, possibly by the X11 header file X.h",
// so this undef is necessary until we can figure out a better solution.
#if defined(Success)
#undef Success
#endif

VRIntroApp::VRIntroApp(bool showMirror) :
  m_HealthWarningDismissed(false),
  m_HelpToggled(false),
  m_OculusMode(true),
  m_ShowMirror(showMirror),
  m_Selected(0),
  m_Zoom(1.0f),
  m_Scale(1.0f) {}

void VRIntroApp::InitMirror() {
#if _WIN32
  if (m_ShowMirror) {
    m_MirrorThread = std::thread(RunMirror, GetHwnd(), std::ref(m_MirrorHWND));
  }
#else
  SDL_Window_ID = this->GetWindowID();
#endif
}

void VRIntroApp::ShutdownMirror() {
  if (m_MirrorThread.joinable()) {
#if _WIN32
    PostMessage(m_MirrorHWND, WM_CLOSE, 0, 0);
#endif
    m_MirrorThread.join();
  }
}

void VRIntroApp::SetFrameSupplier(IFrameSupplier* supplier) {
  m_FrameSupplier = supplier;
}

void VRIntroApp::Initialize() {
  PlatformInitializer init;

  SDLControllerParams params;
  params.transparentWindow = false;
  params.fullscreen = false;
  params.antialias = false;
  params.windowTitle = "Leap Motion VR Intro BETA (F11 to fullscreen)";

  m_Oculus.InitHMD();
  if (!m_Oculus.isDebug()) {
    params.windowWidth = m_Oculus.GetHMDWidth();
    params.windowHeight = m_Oculus.GetHMDHeight();
  } else {
    params.windowWidth = 640;
    params.windowHeight = 720;
  }

  m_applicationTime = TimePoint(0.0);         // Start the application time at zero.

  m_SDLController.Initialize(params);         // This initializes everything SDL-related.
  m_Width = m_SDLController.GetParams().windowWidth;
  m_Height = m_SDLController.GetParams().windowHeight;

  m_GLController.Initialize();                // This initializes the general GL state.
  FreeImage_Initialise();

  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Glew initialization failed");
  }

#if _WIN32
  m_Oculus.SetHWND(m_SDLController.GetHWND());
#endif
  if (!m_Oculus.Init()) {
    throw std::runtime_error("Oculus initialization failed");
  }

  InitMirror();

  // TODO: Add to components
  ovrHmd_RecenterPose(m_Oculus.GetHMD());
  InitializeApplicationLayers();
}

void VRIntroApp::Shutdown() {
  ShutdownApplicationLayers();                // Destroy the application layers, from top (last) to bottom (first).
  ShutdownMirror();

  m_Oculus.Destroy();
  FreeImage_DeInitialise();                   // Shut down FreeImage.
  m_GLController.Shutdown();                  // This shuts down the general GL state.
  m_SDLController.Shutdown();                 // This shuts down everything SDL-related.
}

void VRIntroApp::Update(TimeDelta real_time_delta) {
  PrecisionTimer timer;
  timer.Start();
  assert(real_time_delta >= 0.0);
  m_applicationTime += real_time_delta;         // Increment the application time by the delta.

  m_FrameSupplier->Lock();
  float leap_baseline = m_FrameSupplier->IsDragonfly() ? 64.0f : 40.0f;
  // Set passthrough images
  for (int i = 0; i < 2; i++) {
    m_FrameSupplier->PopulatePassthroughLayer(*m_PassthroughLayer[i], i);
  }

  // Calculate where each point of interest would have to be if a 6.4-cm baseline Leap centered exactly at the eyeballs saw the frame seen. It will be off by a factor of 1.6.
  // This is done using the following formula:
  const float OCULUS_BASELINE = 0.064f; // TODO: Get this value directly from the SDK

  //TODO: This can be obtained by calling ovrHmd_GetSensorState with ScanoutMidpointSeconds for absolute time.
  const EigenTypes::Matrix4x4f avgView = 0.5f*(m_Oculus.EyeView(0) + m_Oculus.EyeView(1));
  EigenTypes::Matrix4x4f inputTransform = avgView.inverse();
  EigenTypes::Matrix3x3f conventionConv;
  conventionConv << -EigenTypes::Vector3f::UnitX(), -EigenTypes::Vector3f::UnitZ(), -EigenTypes::Vector3f::UnitY();
  inputTransform.block<3, 3>(0, 0) *= m_Scale*OCULUS_BASELINE/leap_baseline*conventionConv;

  for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it) {
    InteractionLayer &layer = **it;
    if (layer.Alpha() > 0.01f) {
      m_FrameSupplier->PopulateInteractionLayer(layer, inputTransform.eval().data());
      layer.SetFingerRadius(m_Scale*6.25f*OCULUS_BASELINE/leap_baseline);
      layer.UpdateEyePos(avgView.inverse().block<3, 1>(0, 3));
      layer.UpdateEyeView(avgView.block<3, 3>(0, 0));
      layer.Update(real_time_delta);              // Update each application layer, from back to front.
    }
  }
  m_FrameSupplier->Unlock();

  // Automatically make message and menu disappear after some time
  if (m_applicationTime > 4.0f && !m_HealthWarningDismissed) {
    m_Oculus.DismissHealthWarning();
    m_HealthWarningDismissed = true;
  }

  messageLayer = static_cast<MessageLayer*>(&*m_Layers[MESSAGE_LAYERS]);
  if (m_PassthroughLayer[0]->m_HasData) {
    if (m_applicationTime > 15.0f && !m_HelpToggled) {
      m_HelpToggled = true;
      messageLayer->SetVisible(1, false);
    }
  } else {
    // Leap is not attached or frames are not going through
    messageLayer->SetVisible(0, true);
  }

  messageLayer->SetVisible(2, m_FrameSupplier->GetFPSEstimate() < 59);
  messageLayer->SetVisible(3, m_Oculus.isDebug() && m_OculusMode);

  double elapsed = timer.Stop();
  //std::cout << __LINE__ << ":\t   Update() = " << (elapsed) << std::endl;
}

void VRIntroApp::Render(TimeDelta real_time_delta) const {
  PrecisionTimer timer;
  timer.Start();
  assert(real_time_delta >= 0.0);
  if (m_OculusMode) {
    m_Oculus.BeginFrame();
    glGetError(); // Remove any phantom gl errors before they throw an exception
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Do the eye-order trick!
    for (int i = 1; i >= 0; i--) {
      const ovrRecti& rect = m_Oculus.EyeViewport(i);
      glViewport(rect.Pos.x, rect.Pos.y, rect.Size.w, rect.Size.h);
      RenderEye(real_time_delta, i, m_Oculus.EyeProjection(i));
    }

    double elapsed = timer.Stop();
    //std::cout << __LINE__ << ":\t   Render() = " << (elapsed) << std::endl;
    m_Oculus.EndFrame();
    glGetError(); // Remove any phantom gl errors before they throw an exception
  } else {
    m_SDLController.BeginRender();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_Width, m_Height);

    Projection projection;
    static const double VFOV = 1.0;
    double hfov = VFOV*m_Width/static_cast<double>(m_Height);
    projection.Perspective(-hfov, -VFOV, hfov, VFOV, 0.1, 10000.0);

    RenderEye(real_time_delta, 0, projection.Matrix().cast<float>()); // TODO: Should add an option to oculus vr for eye-agnostic view (halfway between the two eyes)

    double elapsed = timer.Stop();
    //std::cout << __LINE__ << ":\t   Render() = " << (elapsed) << std::endl;
    m_SDLController.EndRender();
  }
}

void VRIntroApp::RenderEye(TimeDelta real_time_delta, int i, const EigenTypes::Matrix4x4f& proj) const {
  const EigenTypes::Matrix4x4f view = m_Oculus.EyeView(i);
  const EigenTypes::Matrix4x4f zoomMat = EigenTypes::Vector4f(m_Zoom, m_Zoom, 1, 1).asDiagonal();

  m_PassthroughLayer[i]->SetProjection(zoomMat*proj);
  m_PassthroughLayer[i]->Render(real_time_delta);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
    // Set individual shader's state
    InteractionLayer &layer = **it;
    if (layer.Alpha() > 0.01f) {
      layer.SetProjection(zoomMat*proj);
      layer.SetModelView(view);
      // Set default shader's state
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(proj.data());
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(view.data());

      layer.Render(real_time_delta);              // Render each application layer, from back to front.
    }
  }
}

EventHandlerAction VRIntroApp::HandleWindowEvent(const SDL_WindowEvent &ev) {
  if (ev.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    m_Width = ev.data1;
    m_Height = ev.data2;

    // Render smoothly in different modes
    glViewport(0,0, m_Width, m_Height);
    m_SDLController.EndRender();
  }
  return DispatchEventToApplicationLayers<SDL_WindowEvent>(ev, &EventHandler::HandleWindowEvent);
}

EventHandlerAction VRIntroApp::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.keysym.sym != SDLK_F11 &&
        ev.keysym.sym != 'o' &&
        m_OculusMode && !m_HealthWarningDismissed) {
      // Any key dismisses the Health and Safety warning on startup
      m_Oculus.DismissHealthWarning();
      m_HealthWarningDismissed = true;
      return EventHandlerAction::CONSUME;
    }
    switch (ev.keysym.sym) {
    case 'o':
      m_OculusMode = !m_OculusMode;
      break;
    case 'h':
      // Hand
      SelectLayer(HAND_LAYER);
      break;
    case SDLK_F1:
      // Help menu
      m_HelpToggled = true;
      {
        messageLayer = static_cast<MessageLayer*>(&*m_Layers[MESSAGE_LAYERS]);
        messageLayer->SetVisible(1, !messageLayer->GetVisible(1));
      }
      break;
    case SDLK_0:
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
      // Content layer
      if (!(SDL_GetModState() & KMOD_CTRL)) {
        for (int i = 0; i < CONTENT_LAYERS; i++) {
          m_Layers[i]->Alpha() = 0;
        }
      }
      // Dim passthrough if in flying stage
      if (ev.keysym.sym != SDLK_0) {
        SelectLayer(ev.keysym.sym - SDLK_1);
      }
      for (int i = 0; i < 2; i++) {
        float alpha;
        if (ev.keysym.sym == SDLK_3) {
          alpha = 0.7f;
        } else if (ev.keysym.sym == SDLK_4) {
          alpha = 0.1f;
        } else {
          alpha = 1.0f;
        }
        m_PassthroughLayer[i]->Alpha() = alpha;
      }
      break;
    case SDLK_UP: {
      float& alpha = m_Layers[m_Selected]->Alpha();
      alpha = std::min(1.0f, alpha + 0.04f);
    }
    break;
    case SDLK_DOWN: {
      float& alpha = m_Layers[m_Selected]->Alpha();
      alpha = std::max(0.0f, alpha - 0.04f);
    }
    break;
    case SDLK_LEFT:
      m_Selected = (m_Selected + m_Layers.size() - 1) % m_Layers.size();
      break;
    case SDLK_RIGHT:
      m_Selected = (m_Selected + 1) % m_Layers.size();
      break;
    case SDLK_F11:
      m_SDLController.ToggleFullscreen();
      //SDL_GetWindowSize(m_SDLController.GetWindow(), &m_Width, &m_Height);
      break;
    case SDLK_HOME:
    case SDLK_F7:
      m_Zoom *= 0.99009901f;
      break;
    case SDLK_END:
    case SDLK_F8:
      m_Zoom *= 1.01f;
      break;
    case SDLK_PAGEUP:
      m_Scale *= 1.01f;
      break;
    case SDLK_PAGEDOWN:
      m_Scale *= 0.99009901f;
      break;
    case SDLK_BACKSPACE:
      m_Zoom = 1.0f;
      m_Scale = 1.0f;
      break;
    case 0x1b:
      exit(0);
    default:
      break;
    }
  }
  return DispatchEventToApplicationLayers<SDL_KeyboardEvent>(ev, &EventHandler::HandleKeyboardEvent);
}

EventHandlerAction VRIntroApp::HandleMouseMotionEvent(const SDL_MouseMotionEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_MouseMotionEvent>(ev, &EventHandler::HandleMouseMotionEvent);
}

EventHandlerAction VRIntroApp::HandleMouseButtonEvent(const SDL_MouseButtonEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_MouseButtonEvent>(ev, &EventHandler::HandleMouseButtonEvent);
}

EventHandlerAction VRIntroApp::HandleMouseWheelEvent(const SDL_MouseWheelEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_MouseWheelEvent>(ev, &EventHandler::HandleMouseWheelEvent);
}

EventHandlerAction VRIntroApp::HandleQuitEvent(const SDL_QuitEvent &ev) {
  return DispatchEventToApplicationLayers<SDL_QuitEvent>(ev, &EventHandler::HandleQuitEvent);
}

EventHandlerAction VRIntroApp::HandleGenericSDLEvent(const SDL_Event &ev) {
  return DispatchEventToApplicationLayers<SDL_Event>(ev, &EventHandler::HandleGenericSDLEvent);
}

TimePoint VRIntroApp::Time() const {
  return m_applicationTime;
}

void VRIntroApp::InitializeApplicationLayers() {
  EigenTypes::Vector3f defaultEyePose(0, 1.675f, -5.f);

  m_Layers.push_back(std::shared_ptr<GridLayer>(new GridLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<SpheresLayer>(new SpheresLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<SpaceLayer>(new SpaceLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<FlyingLayer>(new FlyingLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<FractalLayer>(new FractalLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<QuadsLayer>(new QuadsLayer(defaultEyePose)));

  m_Layers.push_back(std::shared_ptr<HandLayer>(new HandLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<MessageLayer>(new MessageLayer(defaultEyePose)));
  // m_Layers.push_back(std::shared_ptr<MessageLayer>(new MessageLayer(defaultEyePose)));

  m_Layers[CONTENT_LAYERS]->Alpha() = 1;
  m_Layers[CONTENT_LAYERS + 1]->Alpha() = 1;

  for (int i = 0; i < 2; i++) {
    m_PassthroughLayer[i] = std::shared_ptr<PassthroughLayer>(new PassthroughLayer());
    m_PassthroughLayer[i]->Alpha() = 1.0f;
  }
}

void VRIntroApp::ShutdownApplicationLayers() {
  // Destroy the application-specific layers, in reverse order.
  for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
    it->reset();  // destroy the layer by resetting its shared_ptr.
  }
  m_Layers.clear();

  for (int i = 0; i < 2; i++) {
    m_PassthroughLayer[i].reset();
  }
}

void VRIntroApp::SelectLayer(int i) {
  m_Selected = i % m_Layers.size();
  float& alpha = m_Layers[m_Selected]->Alpha();
  alpha = alpha < 0.3f ? 1.0f : 0.0f;
}
