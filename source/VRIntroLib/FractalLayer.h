#pragma once

#include "Interactionlayer.h"

class GLTexture2;

class FractalLayer : public InteractionLayer
{
public:
  FractalLayer(const EigenTypes::Vector3f& initialEyePos);
  //virtual ~FractalLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  mutable GLBuffer m_Buffer;

  std::shared_ptr<GLTexture2> m_Texture;
  TimePoint m_Time;
  
  EigenTypes::Vector3f m_AvgPalm;
};
