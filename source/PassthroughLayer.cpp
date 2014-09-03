#include "PassthroughLayer.h"
#include "GLController.h"
#include "GLShader.h"

PassthroughLayer::PassthroughLayer() :
  InteractionLayer("passthrough"),
  m_image(GLTexture2Params(640, 240, GL_LUMINANCE, GL_UNSIGNED_BYTE), NULL, 640*240),
  m_colorimage(GLTexture2Params(672, 600, GL_RGBA, GL_UNSIGNED_BYTE), NULL, 672*600*4),
  m_distortion(GLTexture2Params(64, 64, GL_RG, GL_FLOAT, GL_RG32F), NULL, 64*64*2),
  m_Gamma(0.8f),
  m_Brightness(1.0f) {
  m_Buffer.Create(GL_ARRAY_BUFFER);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  m_image.Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_image.Unbind();

  m_colorimage.Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_colorimage.Unbind();

  m_distortion.Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_distortion.Unbind();
}

PassthroughLayer::~PassthroughLayer() {
  m_Buffer.Destroy();
}

void PassthroughLayer::SetImage(const unsigned char* data) {
  GLTexture2Params params = m_image.Params();
  m_image.Bind();
  glTexSubImage2D(params.Target(),
                  0,                               // mipmap level (for source images, this should be 0)
                  0, 0,
                  params.Width(),
                  params.Height(),
                  params.PixelDataFormat(),
                  params.PixelDataType(),
                  data);
  m_image.Unbind();
  m_UseColor = false;
}

void PassthroughLayer::SetColorImage(const unsigned char* data) {
  GLTexture2Params params = m_colorimage.Params();
  m_colorimage.Bind();
  glTexSubImage2D(params.Target(),
                  0,                               // mipmap level (for source images, this should be 0)
                  0, 0,
                  params.Width(),
                  params.Height(),
                  params.PixelDataFormat(),
                  params.PixelDataType(),
                  data);
  m_colorimage.Unbind();
  m_UseColor = true;
}

void PassthroughLayer::SetDistortion(const float* data) {
  GLTexture2Params params = m_distortion.Params();
  m_distortion.Bind();
  glTexSubImage2D(params.Target(),
                  0,                               // mipmap level (for source images, this should be 0)
                  0, 0,
                  params.Width(),
                  params.Height(),
                  params.PixelDataFormat(),
                  params.PixelDataType(),
                  data);
  m_distortion.Unbind();
}

void PassthroughLayer::Render(TimeDelta real_time_delta) const {
  m_Shader->Bind();
  m_Renderer.UploadMatrices();

  glActiveTexture(GL_TEXTURE0 + 0);
  if(m_UseColor) {
    m_colorimage.Bind();
  } else {
    m_image.Bind();
  }
  glActiveTexture(GL_TEXTURE0 + 1);
  m_distortion.Bind();

  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUniform2f(m_Shader->LocationOfUniform("ray_scale"), 0.125f, 0.125f);
  glUniform2f(m_Shader->LocationOfUniform("ray_offset"), 0.5f, 0.5f);
  glUniform1i(m_Shader->LocationOfUniform("texture"), 0);
  glUniform1i(m_Shader->LocationOfUniform("distortion"), 1);
  glUniform1f(m_Shader->LocationOfUniform("gamma"), m_Gamma);
  glUniform1f(m_Shader->LocationOfUniform("brightness"), m_Brightness);
  glUniform1f(m_Shader->LocationOfUniform("use_color"), m_UseColor ? 1.0f : 0.0f);

#if 0
  const float edges[] = {-4, -4, -1, -4, 4, -1, 4, -4, -1, 4, 4, -1};
  // Why the fuck doesn't this work?
  m_Renderer.EnablePositionAttribute();
  m_Buffer.Bind();
  m_Buffer.Allocate(&edges[0], sizeof(edges), GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  m_Buffer.Release();
  m_Renderer.DisablePositionAttribute();
#else
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(-4, -4, -1);
  glVertex3f(-4, 4, -1);
  glVertex3f(4, -4, -1);
  glVertex3f(4, 4, -1);
  glEnd();
#endif

  if(m_UseColor) {
    m_colorimage.Unbind();
  } else {
    m_image.Unbind();
  }
  m_distortion.Unbind();
  m_Shader->Unbind();
  glClear(GL_DEPTH_BUFFER_BIT);
}

EventHandlerAction PassthroughLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  case '[':
    m_Gamma = std::max(0.f, m_Gamma - 0.04f);
    return EventHandlerAction::CONSUME;
  case ']':
    m_Gamma = std::min(1.2f, m_Gamma + 0.04f);
    return EventHandlerAction::CONSUME;
  case SDLK_INSERT:
    m_Brightness = std::max(0.f, m_Brightness + 0.03f);
    return EventHandlerAction::CONSUME;
  case SDLK_DELETE:
    m_Brightness = std::min(2.f, m_Brightness - 0.03f);
    return EventHandlerAction::CONSUME;
  default:
    return EventHandlerAction::PASS_ON;
  }
}
