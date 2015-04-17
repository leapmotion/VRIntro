#pragma once

#include "Application.h"
#include "GLController.h"
#include "RenderableEventHandler.h"
#include "SDLController.h"
#include "OculusVR.h"
#include "PassthroughLayer.h"
#include "MessageLayer.h"
#include "RenderState.h"

#include <memory>
#include <vector>
#include <mutex>
#include <thread>

class IFrameSupplier;
class HandLayer;
class MessageLayer;

// Interface class for top-level control of an application.
class VRIntroApp : public Application {
public:
  VRIntroApp(bool showMirror = false);
  // Application interface methods.  See Application for comments and details.

  virtual void Initialize() override;
  virtual void Shutdown() override;
  void InitMirror();
  void ShutdownMirror();

  void SetFrameSupplier(IFrameSupplier* supplier);
  void Run() { RunApplication(*this); }

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  void RenderEye(TimeDelta real_time_delta, int i, const EigenTypes::Matrix4x4f& proj) const;

  virtual EventHandlerAction HandleWindowEvent(const SDL_WindowEvent &ev) override;
  virtual EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;
  virtual EventHandlerAction HandleMouseMotionEvent(const SDL_MouseMotionEvent &ev) override;
  virtual EventHandlerAction HandleMouseButtonEvent(const SDL_MouseButtonEvent &ev) override;
  virtual EventHandlerAction HandleMouseWheelEvent(const SDL_MouseWheelEvent &ev) override;
  virtual EventHandlerAction HandleQuitEvent(const SDL_QuitEvent &ev) override;
  virtual EventHandlerAction HandleGenericSDLEvent(const SDL_Event &ev) override;

  virtual TimePoint Time() const override;

  #if _WIN32
    // TODO: change components so const cast is not required
    HWND GetHwnd() const { return const_cast<SDLController*>(&m_SDLController)->GetHWND(); }
  #elif __APPLE__
    Uint32 GetWindowID() const { return const_cast<SDLController*>(&m_SDLController)->GetWindowID(); }
  #endif

private:
  void InitializeApplicationLayers();
  void ShutdownApplicationLayers();
  void SelectLayer(int i);

  template <typename EventType_>
  EventHandlerAction DispatchEventToApplicationLayers(
    const EventType_ &ev,
    EventHandlerAction(EventHandler::*HandleEvent)(const EventType_ &)) {
    // Give it to the passthrough layers
    for (int i = 0; i < 2; i++) {
      (*(m_PassthroughLayer[i]).*HandleEvent)(ev);
    }

    // Give each application layer a chance to handle the event, stopping if one returns EventHandlerAction::CONSUME.
    for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
      RenderableEventHandler &layer = **it;
      if ((layer.*HandleEvent)(ev) == EventHandlerAction::CONSUME) {
        return EventHandlerAction::CONSUME;
      }
    }
    // No layers consumed the event, so pass it on.
    return EventHandlerAction::PASS_ON;
  }

  mutable OculusVR  m_Oculus;
  SDLController     m_SDLController;
  GLController      m_GLController;
  TimePoint         m_applicationTime;
  std::vector<std::shared_ptr<InteractionLayer>>  m_Layers;
  std::vector<std::shared_ptr<InteractionLayer>>  m_MappedLayers;
  std::shared_ptr<HandLayer> m_HandLayer;
  std::shared_ptr<HandLayer> m_GhostHandLayer;
  std::shared_ptr<MessageLayer> m_MessageLayer;

  std::shared_ptr<PassthroughLayer>               m_PassthroughLayer[2];
  IFrameSupplier*                                 m_FrameSupplier;
  Uint32                                          SDL_Window_ID;
  MessageLayer*                                   messageLayer;
  
  std::thread m_MirrorThread;
#if _WIN32
  HWND m_MirrorHWND;
#endif
  bool m_ShowMirror;

  int  m_Selected;
  bool m_HealthWarningDismissed;
  bool m_HelpToggled;
  bool m_OculusMode;
  bool m_CrippleMode;
  bool m_CroppleMode;
  bool m_UseLatestImage;

  int m_Width;
  int m_Height;

  float m_Zoom;
  float m_Scale;
};
