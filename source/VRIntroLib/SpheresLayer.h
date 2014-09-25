#pragma once

#include "Interactionlayer.h"

class GLShader;

class SpheresLayer : public InteractionLayer {
public:
  SpheresLayer(const EigenTypes::Vector3f& initialEyePos);
  //virtual ~SpheresLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  static const int NUM_SPHERES = 300;

  virtual void RenderGrid() const;
  void ComputePhysics(TimeDelta real_time_delta);

  EigenTypes::stdvectorV3f m_Pos;
  EigenTypes::stdvectorV3f m_Disp;
  EigenTypes::stdvectorV3f m_Vel;
  
  EigenTypes::stdvectorV3f m_Colors;
  EigenTypes::stdvectorV3f m_Mono;
  std::vector<float> m_Radius;

  float m_Spring;
  float m_Damp;
  float m_Well;
};
