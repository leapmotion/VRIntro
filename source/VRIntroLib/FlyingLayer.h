#pragma once

#include "Interactionlayer.h"
#include "WebSocketClient.h"
#include "PlayerState.h"

class GLShader;

class FlyingLayer : public InteractionLayer {
public:
  FlyingLayer(const EigenTypes::Vector3f& initialEyePos, const std::string& server_url = "");
  //virtual ~FlyingLayer ();



  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  static const float X_PERIOD;
  static const float Y_PERIOD;
  static const float Z_PERIOD;

  mutable GLBuffer m_PopupBuffer;
  mutable GLBuffer m_TrailBuffer;
  std::shared_ptr<GLTexture2> m_PopupTexture;
  std::shared_ptr<GLShader> m_PopupShader;
  std::shared_ptr<GLShader> m_AvatarShader;
  std::shared_ptr<GLShader> m_TrailShader;

  void RenderPopup() const;
  void UpdateMultiplayer(TimeDelta real_time_delta);
  void RenderMultiplayer(TimeDelta real_time_delta) const;
  void ReceiveCallback(const std::string& message);
  static Color ColorFromID(int id, float s = 1.0f, float v = 1.0f, float a = 1.0f);

  WebSocketClient m_WebSocketClient;

  EigenTypes::Vector3f m_GridCenter;
  //EigenTypes::Vector3f m_AveragePalm;
  EigenTypes::Vector3f m_Velocity;
  EigenTypes::Vector3f m_RotationAA;
  EigenTypes::Matrix4x4f m_GridOrientation;
  float m_LineThickness;
  int m_GridBrightness;

  EigenTypes::Vector3f m_Center;
  EigenTypes::Matrix3x3f m_Orientation;

  PlayerState m_SelfPlayer;
  mutable std::mutex m_DataMutex;
  std::map<int, PlayerState> m_OtherPlayers;
  std::vector<float> m_TrailAlphas;

  int m_MyID;
};
