#pragma once

#include "Interactionlayer.h"


class PhysicsLayer : public InteractionLayer
{
public:
  PhysicsLayer(const EigenTypes::Vector3f& initialEyePos);
  virtual ~PhysicsLayer ();

  virtual void OnSelected();
  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  class BulletWrapper* m_BulletWrapper;
};
