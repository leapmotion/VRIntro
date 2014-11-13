#pragma once

#include "InteractionLayer.h"

#include "GLTexture2.h"
#include "GLBuffer.h"
#include "EigenTypes.h"

#include <memory>

class GLShader;

class PassthroughLayer : public InteractionLayer {
public:
  PassthroughLayer();
  virtual ~PassthroughLayer();

  void SetImage(const unsigned char* data, int width, int height);
  void SetColorImage(const unsigned char* data);
  void SetDistortion(const float* data);
  void SetCrippleMode(bool value) { m_CrippleMode = value; }
  void SetStencil(bool value) { m_GenerateStencil = value; }

  virtual void Update(TimeDelta real_time_delta) override {}
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;


  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  bool m_HasData;

private:
  void DrawQuad() const;

  mutable GLTexture2 m_image;
  mutable GLTexture2 m_colorimage;
  mutable GLTexture2 m_distortion;
  mutable GLBuffer m_Buffer;
  float m_Gamma;
  float m_Brightness;
  bool m_UseRGBI;
  bool m_GenerateStencil;
  int m_IRMode;
  bool m_CrippleMode;

  // Hack for robust mode
  int m_RealHeight;
};
