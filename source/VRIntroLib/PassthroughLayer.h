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
  //void SetProjection(const EigenTypes::Matrix4x4f& proj) { m_projection = proj; } // HACK until I can do this a better way

  virtual void Update(TimeDelta real_time_delta) override {}
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  bool m_HasData;

private:
  void RenderPopup() const;

  mutable GLBuffer m_PopupBuffer;
  std::shared_ptr<GLTexture2> m_PopupTexture;
  std::shared_ptr<GLShader> m_PopupShader;

  //std::shared_ptr<GLShader> m_shader;
  mutable GLTexture2 m_image;
  mutable GLTexture2 m_colorimage;
  mutable GLTexture2 m_distortion;
  //EigenTypes::Matrix4x4f m_projection
  mutable GLBuffer m_Buffer;
  float m_Gamma;
  float m_Brightness;
  bool m_UseRGBI;
  int m_IRMode;
  bool m_CrippleMode;

  // Hack for robust mode
  int m_RealHeight;
};
