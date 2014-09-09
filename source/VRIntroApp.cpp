#include "VRIntroApp.h"
#include "SpheresLayer.h"
#include "SpaceLayer.h"
#include "HandLayer.h"
#include "GridLayer.h"
#include "MessageLayer.h"
#include "QuadsLayer.h"
#include "FlyingLayer.h"
#include "SDL.h"

#define FREEIMAGE_LIB
#include "FreeImage.h"

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

VRIntroApp::VRIntroApp() :
  m_HealthWarningDismissed(false),
  m_HelpToggled(false),
  m_LeapHMDModeWasOn(false),
  m_OculusMode(true),
  m_Selected(0) {}

void VRIntroApp::Initialize() {
  m_Selected = 0;
  SDLControllerParams params;
  params.transparentWindow = false;
  //params.fullscreen = false;
  params.fullscreen = true;
  params.antialias = false;
  params.windowTitle = "Leap Motion VR Intro BETA (F11 to fullscreen)";

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
  m_LeapController.addListener(m_LeapListener);

  // Temporarily turn on head_mounted_display_mode to on if it was off
  m_LeapHMDModeWasOn = m_LeapController.config().getBool("head_mounted_display_mode");

  int flags = m_LeapController.policyFlags();
  flags |= Leap::Controller::POLICY_IMAGES;

  m_LeapHMDModeWasOn = (flags & Leap::Controller::POLICY_OPTIMIZE_HMD) != 0;
  if (!m_LeapHMDModeWasOn) {
    flags |= Leap::Controller::POLICY_OPTIMIZE_HMD;
  }
  m_LeapController.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>(flags));

  // TODO: Add to components
  ovrHmd_RecenterPose(m_Oculus.GetHMD());
  InitializeApplicationLayers();
}

void VRIntroApp::Shutdown() {
  if (!m_LeapHMDModeWasOn) {
    int flags = m_LeapController.policyFlags();
    flags &= ~Leap::Controller::POLICY_OPTIMIZE_HMD;
    m_LeapController.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>(flags));
  }
  m_LeapController.removeListener(m_LeapListener);

  ShutdownApplicationLayers();                // Destroy the application layers, from top (last) to bottom (first).
  m_GLController.Shutdown();                  // This shuts down the general GL state.
  m_SDLController.Shutdown();                // This shuts down everything SDL-related.
}

void VRIntroApp::Update(TimeDelta real_time_delta) {
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
        m_PassthroughLayer[i]->SetImage(images[i].data(), images[i].width(), images[i].height());
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
      layer.UpdateEyeView(avgView.block<3, 3>(0, 0));
      layer.Update(real_time_delta);              // Update each application layer, from back to front.
    }
  }

  // Automatically make message and menu disappear after some time
  if (m_applicationTime > 4.0f && !m_HealthWarningDismissed) {
    m_Oculus.DismissHealthWarning();
    m_HealthWarningDismissed = true;
  }
  if (m_applicationTime > 15.0f && !m_HelpToggled) {
    m_HelpToggled = true;
    MessageLayer* messageLayer = static_cast<MessageLayer*>(&*m_Layers[HELP_LAYER]);
    messageLayer->SetVisible(0, false);
  }
}

void VRIntroApp::Render(TimeDelta real_time_delta) const {
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
    m_SDLController.EndRender();
  }
  return;
}

void VRIntroApp::RenderEye(TimeDelta real_time_delta, int i, const Matrix4x4f& proj) const {
  const Matrix4x4f view = m_Oculus.EyeView(i);

  m_PassthroughLayer[i]->SetProjection(proj);
  m_PassthroughLayer[i]->Render(real_time_delta);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it) {
    // Set individual shader's state
    InteractionLayer &layer = **it;
    if (layer.Alpha() > 0.01f) {
      layer.SetProjection(proj);
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
        MessageLayer* messageLayer = static_cast<MessageLayer*>(&*m_Layers[HELP_LAYER]);
        messageLayer->SetVisible(0, !messageLayer->GetVisible(0));
      }
      break;
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
      // Content layer
      if (!(SDL_GetModState() & KMOD_CTRL)) {
        for (int i = 0; i < CONTENT_LAYERS; i++) {
          m_Layers[i]->Alpha() = 0;
        }
      }
      // Dim passthrough if in flying stage
      SelectLayer(ev.keysym.sym - SDLK_1);
      for (int i = 0; i < 2; i++) {
        float& alpha = m_PassthroughLayer[i]->Alpha() = (ev.keysym.sym - SDLK_1 == 3) ? 0.1f : 1.0f;
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
  exit(0);
}

EventHandlerAction VRIntroApp::HandleGenericSDLEvent(const SDL_Event &ev) {
  return DispatchEventToApplicationLayers<SDL_Event>(ev, &EventHandler::HandleGenericSDLEvent);
}

TimePoint VRIntroApp::Time() const {
  return m_applicationTime;
}

void VRIntroApp::InitializeApplicationLayers() {
  Vector3f defaultEyePose(0, 1.675f, -5.f);

  m_Layers.push_back(std::shared_ptr<GridLayer>(new GridLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<SpheresLayer>(new SpheresLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<SpaceLayer>(new SpaceLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<FlyingLayer>(new FlyingLayer(defaultEyePose)));

  m_Layers.push_back(std::shared_ptr<HandLayer>(new HandLayer(defaultEyePose)));
  m_Layers.push_back(std::shared_ptr<MessageLayer>(new MessageLayer(defaultEyePose)));
  // m_Layers.push_back(std::shared_ptr<MessageLayer>(new MessageLayer(defaultEyePose)));
  //m_Layers.push_back(std::shared_ptr<QuadsLayer>(new QuadsLayer(defaultEyePose)));

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
