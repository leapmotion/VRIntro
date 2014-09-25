#pragma once

#include "Interactionlayer.h"

class GLShader;

class GridLayer : public InteractionLayer {
public:
  GridLayer(const EigenTypes::Vector3f& initialEyePos);
  //virtual ~GridLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
  int m_DivTheta;
  int m_DivPhi;
  float m_Radius;
};
