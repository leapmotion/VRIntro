#pragma once

#include "Interactionlayer.h"

class GLShader;

class HandLayer : public InteractionLayer {
public:
  HandLayer(const Vector3f& initialEyePos);
  //virtual ~HandLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
};
