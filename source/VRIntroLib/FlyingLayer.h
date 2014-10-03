#pragma once

#include "Interactionlayer.h"

class GLShader;

class FlyingLayer : public InteractionLayer {
public:
  FlyingLayer(const EigenTypes::Vector3f& initialEyePos);
  //virtual ~FlyingLayer ();



  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  mutable GLBuffer m_PopupBuffer;
  std::shared_ptr<GLTexture2> m_PopupTexture;
  std::shared_ptr<GLShader> m_PopupShader;

  void RenderPopup() const;

  EigenTypes::Vector3f m_GridCenter;
  //EigenTypes::Vector3f m_AveragePalm;
  EigenTypes::Vector3f m_Velocity;
  EigenTypes::Vector3f m_RotationAA;
  EigenTypes::Matrix4x4f m_GridOrientation;
  float m_LineThickness;
  int m_GridBrightness;
};
