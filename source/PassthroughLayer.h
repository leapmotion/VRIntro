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

  void SetImage(const unsigned char* data);
  void SetColorImage(const unsigned char* data);
  void SetDistortion(const float* data);
  //void SetProjection(const Matrix4x4f& proj) { m_projection = proj; } // HACK until I can do this a better way

  virtual void Update(TimeDelta real_time_delta) override {}
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction PassthroughLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  //std::shared_ptr<GLShader> m_shader;
  mutable GLTexture2 m_image;
  mutable GLTexture2 m_colorimage;
  mutable GLTexture2 m_distortion;
  //Matrix4x4f m_projection
  mutable GLBuffer m_Buffer;
  float m_Gamma;
  float m_Brightness;
  bool m_UseColor;
};
