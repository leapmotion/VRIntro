#pragma once

#include "Interactionlayer.h"

class GLShader;

class QuadsLayer : public InteractionLayer {
public:
  QuadsLayer(const EigenTypes::Vector3f& initialEyePos);
  //virtual ~QuadsLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
};
