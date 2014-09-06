#pragma once

#include "Interactionlayer.h"

class GLTexture2;

class HelpLayer : public InteractionLayer {
public:
  HelpLayer(const Vector3f& initialEyePos);
  //virtual ~HelpLayer ();

  virtual void Update(TimeDelta real_time_delta) override {}
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  void SetVisible(int index, bool value) { m_Visible[index] = value; }
  bool GetVisible(int index) const { return m_Visible[index]; }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  static const int NUM_MESSAGES = 2;
  void DrawMessage(int index) const;

  std::shared_ptr<GLTexture2> m_HelpTexture;
  std::shared_ptr<GLTexture2> m_LowFPSTexture;

  bool m_Visible[NUM_MESSAGES];
};
