#pragma once

#include "Interactionlayer.h"

class GLTexture2;

class HelpLayer : public InteractionLayer {
public:
  HelpLayer(const Vector3f& initialEyePos);
  //virtual ~HelpLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  std::shared_ptr<GLTexture2> m_texture;
};
